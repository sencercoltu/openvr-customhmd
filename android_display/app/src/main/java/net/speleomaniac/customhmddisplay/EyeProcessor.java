package net.speleomaniac.customhmddisplay;

class EyeProcessor {
    EyeProcessor(DisplayActivity activity) {
        _activity = activity;
    }
    EyeProcessorThread _processor;
    Thread _thread;
    DisplayActivity _activity;
}
