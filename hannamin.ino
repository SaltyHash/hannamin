/* Code for the Hannamin. This is meant to run on a PJRC Teensy 4.0 microcontroller. */


#include <Adafruit_VL53L0X.h>
#include <Audio.h>

//#define HANNAMIN_DEBUG


// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine4; //xy=395,468
AudioSynthWaveformSine   sine1;          //xy=396,324
AudioSynthWaveformSine   sine2; //xy=396,371
AudioSynthWaveformSine   sine3;   //xy=396,421
AudioMixer4              mixer1;         //xy=589,344
AudioSynthWaveformDc     master_volume; //xy=718,410
AudioFilterBiquad        filter;        //xy=740,344
AudioEffectMultiply      multiply1;      //xy=899,381
AudioAmplifier           i2s_amp;           //xy=1061,380
AudioOutputI2S           i2s;           //xy=1232,381
AudioConnection          patchCord1(sine4, 0, mixer1, 3);
AudioConnection          patchCord2(sine1, 0, mixer1, 0);
AudioConnection          patchCord3(sine2, 0, mixer1, 1);
AudioConnection          patchCord4(sine3, 0, mixer1, 2);
AudioConnection          patchCord5(mixer1, filter);
AudioConnection          patchCord6(master_volume, 0, multiply1, 1);
AudioConnection          patchCord7(filter, 0, multiply1, 0);
AudioConnection          patchCord8(multiply1, i2s_amp);
AudioConnection          patchCord9(i2s_amp, 0, i2s, 0);
AudioConnection          patchCord10(i2s_amp, 0, i2s, 1);
// GUItool: end automatically generated code


const int LED = 13;
const int MIN_DIST_MM = 80;
const int RANGE_MM = 350;
const float MIN_PITCH = 220;
const float MAX_PITCH = MIN_PITCH * 2 * 2 * 2;

const int MIDI_CHANNEL = 0;
const int MIDI_VELOCITY = 100;
const int MIDI_CONTROL_CHANGE_CHANNEL_VOLUME = 0x07;
const int MIDI_CONTROL_CHANGE_ALL_NOTES_OFF  = 0x7B;

Adafruit_VL53L0X left_sensor = Adafruit_VL53L0X();


void setup() {
#ifdef HANNAMIN_DEBUG
  Serial.begin(115200);
#endif

  pinMode(LED, OUTPUT);

  // Setup hand sensors
  while (!left_sensor.begin(0x30)) {
#ifdef HANNAMIN_DEBUG
    Serial.println("ERROR: Failed to initialize left sensor!");
#endif
    delay(1000);
  }


  // Setup audio stuff
  AudioMemory(10);

  sine1.amplitude(0.5);
  sine2.amplitude(0.5 * 0.5);
  sine3.amplitude(0.5 * pow(0.5, 5));
  sine4.amplitude(0.5 * pow(0.5, 7));

//  filter.setNotch(0, 200, 0.5);
  filter.setLowpass(0, 2000, 0.7);

  set_volume(1.0);
}

void loop() {
  static int last_distance = -1;

#ifdef HANNAMIN_DEBUG
  Serial.print("CPU:");
  Serial.print(AudioProcessorUsage());
  Serial.print(", CPU_MAX:");
  Serial.print(AudioProcessorUsageMax());
  Serial.print(", MEM:");
  Serial.print(AudioMemoryUsage());
  Serial.print(", MEM_MAX:");
  Serial.print(AudioMemoryUsageMax());
  Serial.print("\n");
#endif

  VL53L0X_RangingMeasurementData_t measure;
  left_sensor.rangingTest(&measure);
  const int left_distance = measure.RangeStatus != 4 ? measure.RangeMilliMeter : -1;
  digitalWrite(LED, left_distance >= 0);

#ifdef HANNAMIN_DEBUG
  Serial.printf("left_distance = %d mm\n", left_distance);
#endif

  if (left_distance == -1) {
    if (last_distance != -1) {
      set_volume(0.0);
    }

    last_distance = left_distance;

    return;
  } else {
    if (last_distance == -1) {
      set_volume(1.0);
    }
  }

  if (last_distance == -1) {
    last_distance = left_distance;
  }
  const float alpha = 0.6;
  const int distance = alpha * left_distance + (1.0 - alpha) * last_distance;
  last_distance = distance;

  float scalar = 1.0 - min(max(0.0, (float)(distance - MIN_DIST_MM) / RANGE_MM), 1.0);

#ifdef HANNAMIN_DEBUG
  Serial.printf("scalar = %.2f\n", scalar);
#endif

  const float pitch = MIN_PITCH + (MAX_PITCH - MIN_PITCH) * pow(scalar, 2);
  set_pitch(pitch);

  // Discard all MIDI messages
  while (usbMIDI.read());
}

void set_pitch(const float pitch) {
  static int last_root_midi_note = -1;

  AudioNoInterrupts();
  sine1.frequency(pitch);
  sine2.frequency(pitch * 2);
  sine3.frequency(pitch * 3);
  sine4.frequency(pitch * 4);
  AudioInterrupts();

  // Convert the frequency into a MIDI note and pitch bend
  // MIDI_note = 12 * log2(pitch / 440) + 69
  //           = a * log(pitch / 440) + 69
  // where   a = 12 / log(2)
  const static float a = 12.0 / log(2);
  const float midi_note = a * log(pitch / 440) + 69;
  const int root_midi_note = (int) round(midi_note);
  if (root_midi_note != last_root_midi_note) {
//    usbMIDI.sendControlChange(MIDI_CONTROL_CHANGE_ALL_NOTES_OFF, 0, MIDI_CHANNEL);
    if (last_root_midi_note != -1) {
      usbMIDI.sendNoteOff(last_root_midi_note, MIDI_VELOCITY, MIDI_CHANNEL);
    }
    usbMIDI.sendNoteOn(root_midi_note, MIDI_VELOCITY, MIDI_CHANNEL);
    last_root_midi_note = root_midi_note;
  }
  // TODO: Pitch bending

#ifdef HANNAMIN_DEBUG
  Serial.printf("pitch = %d Hz\n", (int) pitch);
  Serial.printf("MIDI note = %d\n", (int) midi_note);
#endif
}

void set_volume(const float level) {
  if (master_volume.read() != level) {
    master_volume.amplitude(level, 10);

    const int midi_volume = min(max(0, round(level * 127)), 127);
    usbMIDI.sendControlChange(MIDI_CONTROL_CHANGE_CHANNEL_VOLUME, midi_volume, MIDI_CHANNEL);

#ifdef HANNAMIN_DEBUG
    Serial.printf("volume level = %.2f\n", level);
    Serial.printf("MIDI volume  = %d\n", midi_volume);
#endif
  }
}

bool almost_equal(const float a, const float b) {
  return abs(a - b) < 0.000001;
}
