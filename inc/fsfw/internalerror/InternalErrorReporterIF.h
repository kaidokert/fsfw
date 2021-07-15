#ifndef INTERNALERRORREPORTERIF_H_
#define INTERNALERRORREPORTERIF_H_

class InternalErrorReporterIF {
public:
	virtual ~InternalErrorReporterIF() {
	}

	/**
	 * Thread safe
	 */
	virtual void queueMessageNotSent() = 0;

	/**
	 * Thread safe
	 */
	virtual void lostTm() = 0;

	/**
	 * Thread safe
	 */
	virtual void storeFull() = 0;


};

#endif /* INTERNALERRORREPORTERIF_H_ */
