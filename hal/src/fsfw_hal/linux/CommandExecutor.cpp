#include "CommandExecutor.h"

#include "fsfw/serviceinterface.h"
#include "fsfw/container/SimpleRingBuffer.h"
#include "fsfw/container/DynamicFIFO.h"

#include <unistd.h>

#include <cstring>

CommandExecutor::CommandExecutor(const size_t maxSize):
readVec(maxSize) {
    waiter.events = POLLIN;
}

ReturnValue_t CommandExecutor::load(std::string command, bool blocking, bool printOutput) {
    if(state == States::PENDING) {
        return COMMAND_PENDING;
    }

    currentCmd = command;
    this->blocking = blocking;
    this->printOutput = printOutput;
    if(state == States::IDLE) {
        state = States::COMMAND_LOADED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CommandExecutor::execute() {
    if(state == States::IDLE) {
        return NO_COMMAND_LOADED_OR_PENDING;
    }
    else if(state == States::PENDING) {
        return COMMAND_PENDING;
    }
    currentCmdFile = popen(currentCmd.c_str(), "r");
    if(currentCmdFile == nullptr) {
        lastError = errno;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    if(blocking) {
        ReturnValue_t result = executeBlocking();
        state = States::IDLE;
        return result;
    }
    else {
        currentFd = fileno(currentCmdFile);
        waiter.fd = currentFd;
    }
    state = States::PENDING;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CommandExecutor::close() {
    if(state == States::PENDING) {
        // Attempt to close process, irrespective of if it is running or not
        if(currentCmdFile != nullptr) {
            pclose(currentCmdFile);
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void CommandExecutor::printLastError(std::string funcName) const {
    if(lastError != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << funcName << " pclose failed with code " << lastError << ": " <<
                strerror(lastError) << std::endl;
#else
        sif::printError("%s pclose failed with code %d: %s\n",
                funcName, lastError, strerror(lastError));
#endif
    }
}

void CommandExecutor::setRingBuffer(SimpleRingBuffer *ringBuffer,
        DynamicFIFO<uint16_t>* sizesFifo) {
    this->ringBuffer = ringBuffer;
    this->sizesFifo = sizesFifo;
}

ReturnValue_t CommandExecutor::check(bool& replyReceived) {
    if(blocking) {
        return HasReturnvaluesIF::RETURN_OK;
    }
    switch(state) {
    case(States::IDLE):
    case(States::COMMAND_LOADED): {
        return NO_COMMAND_LOADED_OR_PENDING;
    }
    case(States::PENDING): {
        break;
    }
    }

    int result = poll(&waiter, 1, 0);
    switch(result) {
    case(0): {
        return HasReturnvaluesIF::RETURN_OK;
        break;
    }
    case(1): {
        if (waiter.revents & POLLIN) {
            ssize_t readBytes = read(currentFd, readVec.data(), readVec.size());
            if(readBytes == 0) {
                // Should not happen
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::warning << "CommandExecutor::check: No bytes read "
                        "after poll event.." << std::endl;
#else
                sif::printWarning("CommandExecutor::check: No bytes read after poll event..\n");
#endif
                break;
            }
            else if(readBytes > 0) {
                replyReceived = true;
                if(printOutput) {
                    // It is assumed the command output is line terminated
#if FSFW_CPP_OSTREAM_ENABLED == 1
                    sif::info << currentCmd << " | " << readVec.data();
#else
                    sif::printInfo("%s | %s", currentCmd, readVec.data());
#endif
                }
                if(ringBuffer != nullptr) {
                    ringBuffer->writeData(reinterpret_cast<const uint8_t*>(
                            readVec.data()), readBytes);
                }
                if(sizesFifo != nullptr) {
                    if(not sizesFifo->full()) {
                        sizesFifo->insert(readBytes);
                    }
                }
            }
            else {
                // Should also not happen
#if FSFW_CPP_OSTREAM_ENABLED == 1
                sif::warning << "CommandExecutor::check: Error " << errno << ": " <<
                        strerror(errno) << std::endl;
#else
                sif::printWarning("CommandExecutor::check: Error %d: %s\n", errno, strerror(errno));
#endif
            }
        }
        if(waiter.revents & POLLERR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::warning << "CommandExecuter::check: Poll error" << std::endl;
#else
            sif::printWarning("CommandExecuter::check: Poll error\n");
#endif
            return COMMAND_ERROR;
        }
        if(waiter.revents & POLLHUP) {
            result = pclose(currentCmdFile);
            ReturnValue_t retval = EXECUTION_FINISHED;
            if(result != 0) {
                lastError = result;
                retval = HasReturnvaluesIF::RETURN_FAILED;
            }
            state = States::IDLE;
            currentCmdFile = nullptr;
            currentFd = 0;
            return retval;
        }
        break;
    }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void CommandExecutor::reset() {
    CommandExecutor::close();
    currentCmdFile = nullptr;
    currentFd = 0;
    state = States::IDLE;
}

int CommandExecutor::getLastError() const {
    // See: https://stackoverflow.com/questions/808541/any-benefit-in-using-wexitstatus-macro-in-c-over-division-by-256-on-exit-statu
    return WEXITSTATUS(this->lastError);
}

CommandExecutor::States CommandExecutor::getCurrentState() const {
    return state;
}

ReturnValue_t CommandExecutor::executeBlocking() {
    while(fgets(readVec.data(), readVec.size(), currentCmdFile) != nullptr) {
        std::string output(readVec.data());
        if(printOutput) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::info << currentCmd << " | " << output;
#else
            sif::printInfo("%s | %s", currentCmd, output);
#endif
        }
        if(ringBuffer != nullptr) {
            ringBuffer->writeData(reinterpret_cast<const uint8_t*>(output.data()), output.size());
        }
        if(sizesFifo != nullptr) {
            if(not sizesFifo->full()) {
                sizesFifo->insert(output.size());
            }
        }
    }
    int result = pclose(currentCmdFile);
    if(result != 0) {
        lastError = result;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}
