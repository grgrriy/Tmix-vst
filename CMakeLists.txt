cmake_minimum_required(VERSION 3.22)
project(TMix VERSION 0.1.0)

# Set JUCE_DIR to your local JUCE checkout when configuring CMake.
add_subdirectory(JUCE)

juce_add_plugin(TMix
    COMPANY_NAME "T-Mix Audio"
    BUNDLE_ID com.tmixaudio.tmix
    PLUGIN_MANUFACTURER_CODE TMix
    PLUGIN_CODE Tm01
    FORMATS VST3 AU
    PRODUCT_NAME "T-Mix"
    IS_SYNTH FALSE
    NEEDS_MIDI FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(TMix PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_features(TMix PRIVATE cxx_std_17)
target_link_libraries(TMix PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
)
target_compile_definitions(TMix PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
