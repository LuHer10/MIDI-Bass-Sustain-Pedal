/*
 * midi_bass_sustain.ino
 *
 * Created: 12/11/2025
 * Author: Lucas Herrera
 */ 

#include "MIDIUSB.h"

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

int toggle_pin = 8;
int toggle_pressed = 0;
int pre_toggle_pressed = 0;
int sust_toggled = 0;

int sust_pin = 7;
int sust_pressed = 0;
int pre_sust_pressed = 0;

midiEventPacket_t rx;
midiEventPacket_t pre_rx;

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  Serial.begin(115200);
  pinMode(toggle_pin, INPUT);
  pinMode(sust_pin, INPUT);
  rx = MidiUSB.read();
  pre_rx = MidiUSB.read();
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void loop() {
  pre_sust_pressed = sust_pressed;
  sust_pressed = digitalRead(sust_pin);

  pre_toggle_pressed = toggle_pressed;
  toggle_pressed = digitalRead(toggle_pin);

  if(pre_toggle_pressed != toggle_pressed && toggle_pressed)
  {
    sust_toggled = 1 - sust_toggled;
    Serial.print("Toggle state: ");
    Serial.println(sust_toggled);
    controlChange(0, 64, 127*sust_toggled);
    MidiUSB.flush();
  }

    if(pre_sust_pressed != sust_pressed && sust_pressed)
    {
      controlChange(0, 64, 127*(1-sust_toggled));
      MidiUSB.flush();
      Serial.println("sust on");
    }
    if(pre_sust_pressed != sust_pressed && !sust_pressed)
    {
      controlChange(0, 64, 127*sust_toggled);
      MidiUSB.flush();
      Serial.println("sust off");
    }

  do {
    rx = MidiUSB.read();
    if(sust_toggled && rx.header == 0x09)// && rx.byte2 != pre_rx.byte2)
    {
      if(pre_rx.byte1 == rx.byte1 && pre_rx.byte2 == rx.byte2)
      {
        //controlChange(0, 64, 127);
        //MidiUSB.flush();
      }
      else
      {
        controlChange(0, 64, 0);
        MidiUSB.flush();
        controlChange(0, 64, 127);
        MidiUSB.flush();
      }
    }
    if(rx.header == 0x09)
      pre_rx = rx;
  } while (MidiUSB.available() > 0);

}
