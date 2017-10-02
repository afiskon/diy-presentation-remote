TARGET := main
SOURCES := main.cpp $(ARDUINODIR)/libraries/Keyboard/src/Keyboard.cpp $(ARDUINODIR)/hardware/arduino/avr/libraries/HID/src/HID.cpp
CPPFLAGS += -I $(ARDUINODIR)/libraries/Keyboard/src -I $(ARDUINODIR)/hardware/arduino/avr/libraries/HID/src
BOARD := leonardo
include arduino.mk
