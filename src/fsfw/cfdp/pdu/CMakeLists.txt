target_sources(${LIB_FSFW_NAME} PRIVATE
    PduConfig.cpp
    VarLenField.cpp
    HeaderSerializer.cpp
    HeaderDeserializer.cpp
    FileDirectiveDeserializer.cpp
    FileDirectiveSerializer.cpp
 
    AckInfo.cpp
    AckPduSerializer.cpp
    AckPduDeserializer.cpp
    EofInfo.cpp
    EofPduSerializer.cpp
    EofPduDeserializer.cpp
    NakInfo.cpp
    NakPduSerializer.cpp
    NakPduDeserializer.cpp
    FinishedInfo.cpp
    FinishedPduSerializer.cpp
    FinishedPduDeserializer.cpp
    MetadataInfo.cpp
    MetadataPduSerializer.cpp
    MetadataPduDeserializer.cpp
    KeepAlivePduSerializer.cpp
    KeepAlivePduDeserializer.cpp
    PromptPduSerializer.cpp
    PromptPduDeserializer.cpp

    FileDataSerializer.cpp
    FileDataDeserializer.cpp
    FileDataInfo.cpp
)