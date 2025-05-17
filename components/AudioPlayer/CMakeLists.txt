idf_component_register(SRCS "AudioPlayer.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
    audio_pipeline
    esp_peripherals
    audio_stream
    )
