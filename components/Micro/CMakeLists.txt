idf_component_register(SRCS "Micro.cpp" "Vad.cpp" "Recorder.cpp"
    INCLUDE_DIRS "include"
    REQUIRES audio_pipeline
    esp_peripherals
    audio_stream
    )
