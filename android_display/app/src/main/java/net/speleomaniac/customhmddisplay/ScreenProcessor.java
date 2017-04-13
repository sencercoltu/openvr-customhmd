package net.speleomaniac.customhmddisplay;

class ScreenProcessor {
    ScreenProcessor(DisplayActivity activity) {
        _activity = activity;
    }
    ScreenProcessorThread _processor;
    Thread _thread;
    DisplayActivity _activity;
}
