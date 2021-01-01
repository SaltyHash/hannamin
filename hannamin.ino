/* Code for the Hannamin. This is meant to run on a PJRC Teensy 4.0 microcontroller. */


#include <Adafruit_VL53L0X.h>
#include <Audio.h>

//#define HANNAMIN_DEBUG


// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=268,323
AudioEffectRectifier     rectify1;       //xy=429,323
AudioSynthWaveformDc     dc1;            //xy=429,366
AudioMixer4              mixer1;         //xy=589,344
AudioSynthWaveformDc     master_volume; //xy=718,410
AudioFilterBiquad        filter;        //xy=740,344
AudioEffectMultiply      multiply1;      //xy=899,381
AudioAmplifier           i2s_amp;           //xy=1061,380
AudioOutputI2S           i2s;           //xy=1232,381
AudioConnection          patchCord1(sine1, rectify1);
AudioConnection          patchCord2(rectify1, 0, mixer1, 0);
AudioConnection          patchCord3(dc1, 0, mixer1, 3);
AudioConnection          patchCord4(mixer1, filter);
AudioConnection          patchCord5(master_volume, 0, multiply1, 1);
AudioConnection          patchCord6(filter, 0, multiply1, 0);
AudioConnection          patchCord7(multiply1, i2s_amp);
AudioConnection          patchCord8(i2s_amp, 0, i2s, 0);
AudioConnection          patchCord9(i2s_amp, 0, i2s, 1);
// GUItool: end automatically generated code


const int LED = 13;
const int MIN_DIST_MM = 80;
const int RANGE_MM = 350;
const float MIN_PITCH = 110;
const float MAX_PITCH = MIN_PITCH * 2 * 2;

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
  AudioMemory(20);

  sine1.amplitude(1.0);

  dc1.amplitude(-0.5);

//  filter.setNotch(0, 200, 0.5);
  filter.setLowpass(0, 1000, 0.7);

  set_volume(1.0);
}

void loop() {
  static int last_distance = -1;

  VL53L0X_RangingMeasurementData_t measure;
  left_sensor.rangingTest(&measure);
  const int left_distance = measure.RangeStatus != 4 ? measure.RangeMilliMeter : -1;
  digitalWrite(LED, left_distance >= 0);

#ifdef HANNAMIN_DEBUG
  Serial.printf("left_distance = %d mm\n", left_distance);
#endif

  if (left_distance < 0) {
    set_volume(0.0);
    return;
  } else {
    set_volume(1.0);
  }

  if (last_distance == -1) {
    last_distance = left_distance;
  }
  const float alpha = 0.9;
  const int distance = alpha * left_distance + (1.0 - alpha) * last_distance;
  last_distance = distance;

  float scalar = 1.0 - min(max(0.0, (float)(distance - MIN_DIST_MM) / RANGE_MM), 1.0);
  scalar = pow(scalar, 2);

#ifdef HANNAMIN_DEBUG
  Serial.printf("scalar = %.2f\n", scalar);
#endif

  const float pitch  = MIN_PITCH + (MAX_PITCH - MIN_PITCH) * scalar;

#ifdef HANNAMIN_DEBUG
  Serial.printf("pitch = %d Hz\n", (int) pitch);
#endif

  set_pitch(pitch);
}

void set_pitch(const float pitch) {
  sine1.frequency(pitch / 2);
}

void set_volume(const float level) {
  if (master_volume.read() != level) {
    master_volume.amplitude(level, 10);
  }
}
