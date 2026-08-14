cmake_minimum_required(VERSION 3.22)
project(SordIR VERSION 1.5.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()
if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "" FORCE)
endif()

add_subdirectory(JUCE)

juce_add_binary_data(SordIR_BinaryData
    SOURCES
        Resources/IRs/Trpt_Straight.wav
        Resources/IRs/Trpt_Bucket.wav
        Resources/IRs/Trpt_Cup.wav
        Resources/IRs/Trpt_Harmon.wav
        Resources/IRs/Trom_Straight.wav
        Resources/IRs/Trom_Bucket.wav
        Resources/IRs/Trom_Cup.wav
        Resources/IRs/Trom_Harmon.wav
        Resources/IRs/Trom_Harmon_Stem.wav
        Resources/IRs/Horn_Straight.wav
        Resources/IRs/Horn_Brass_Stop.wav
        Resources/IRs/Tuba_Straight.wav
        Resources/IRs/Sordino_1.wav
        Resources/IRs/IR_to_create_depth.wav
        Resources/IRs/AIR-STUDIOS-IR-BRASS.wav
        Resources/IRs/Bricasti_Large_Hall.wav
        Resources/IRs/Bricasti_Brass_Hall.wav
        Resources/IRs/Bricasti_Amsterdam.wav
        Resources/IRs/Bricasti_Boston.wav
        Resources/IRs/Bricasti_Worcester.wav
        Resources/IRs/Bricasti_Mechanics.wav
        Resources/IRs/Bricasti_Saint_Gerold.wav
)

juce_add_plugin(SordIR
    COMPANY_NAME         "Sord"
    PLUGIN_MANUFACTURER_CODE Sord
    PLUGIN_CODE          SdIR
    FORMATS              VST3 AU Standalone
    PRODUCT_NAME         "Sord IR"
    IS_SYNTH             FALSE
    NEEDS_MIDI_INPUT     FALSE
    NEEDS_MIDI_OUTPUT    FALSE
    IS_MIDI_EFFECT       FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES      "Fx" "Reverb"
    AU_MAIN_TYPE         kAudioUnitType_Effect
)

target_sources(SordIR PRIVATE
    Source/ParamIDs.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ConvolutionEngine.cpp
    Source/EQProcessor.cpp
    Source/LookAndFeel.cpp
)

target_compile_definitions(SordIR PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(SordIR PRIVATE
    SordIR_BinaryData
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
