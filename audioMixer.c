// Incomplete implementation of an audio mixer. Search for "REVISIT" to find things
// which are left as incomplete.
// Note: Generates low latency audio on BeagleBone Black; higher latency found on host.
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h> // needed for mixer

#include "audioMixer.h"

#define AUD_DRUM_HIGHHAT "wave-files/high_hat_closed.wav"
#define AUD_DRUM_BASS "wave-files/bass_hard.wav"
#define AUD_DRUM_SNARE "wave-files/snare_hard.wav"

static snd_pcm_t *handle;

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define SAMPLE_SIZE (sizeof(short)) 			// bytes per sample
// Sample size note: This works for mono files because each sample ("frame') is 1 value.
// If using stereo files then a frame would be two samples.

static unsigned long playbackBufferSize = 0;
static short *playbackBuffer = NULL;


// Currently active (waiting to be played) sound bites
#define MAX_SOUND_BITES 30
typedef struct {
	// A pointer to a previously allocated sound bite (wavedata_t struct).
	// Note that many different sound-bite slots could share the same pointer
	// (overlapping cymbal crashes, for example)
	wavedata_t *pSound;

	// The offset into the pData of pSound. Indicates how much of the
	// sound has already been played (and hence where to start playing next).
	int location;
} playbackSound_t;
static playbackSound_t soundBites[MAX_SOUND_BITES];
static pthread_mutex_t mutSoundBites = PTHREAD_MUTEX_INITIALIZER;
static int freeSpaceCursor = 0;

// Playback threading
void* playbackThread(void* arg);
static _Bool stopping = false;
static pthread_t playbackThreadId;

// Beat threading
void* enqueueBeatThread(void* arg);
static pthread_t enqueueBeatThreadId;

enum beat currentBeat = BEAT1;
static pthread_mutex_t mutBeat = PTHREAD_MUTEX_INITIALIZER;

struct timespec beatDelay;

wavedata_t dBass;
wavedata_t dHH;
wavedata_t dSnare;
static void playBeat1(int beatCounter);
static void playBeat2(int beatCounter);

static pthread_mutex_t mutVolume = PTHREAD_MUTEX_INITIALIZER;
static int volume = DEFAULT_VOLUME;

static pthread_mutex_t mutBPM = PTHREAD_MUTEX_INITIALIZER;
static int bpm = DEFAULT_BPM;


void AudioMixer_init(void) {
	AudioMixer_setVolume(DEFAULT_VOLUME);

	// Initialize the currently active sound-bites being played
	// REVISIT:- Implement this. Hint: set the pSound pointer to NULL for each
	//     sound bite.

	for (int i = 0; i < MAX_SOUND_BITES; i++) {
		soundBites[i].pSound = NULL;
		soundBites[i].location = 0;
	}

	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Configure parameters of PCM output
	err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			NUM_CHANNELS,
			SAMPLE_RATE,
			1,			// Allow software resampling
			50000);		// 0.05 seconds per buffer
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Allocate this software's playback buffer to be the same size as the
	// the hardware's playback buffers for efficient data transfers.
	// ..get info on the hardware buffers:
 	unsigned long unusedBufferSize = 0;
	snd_pcm_get_params(handle, &unusedBufferSize, &playbackBufferSize);
	// ..allocate playback buffer:
	playbackBuffer = malloc(playbackBufferSize * sizeof(*playbackBuffer));

	// Launch playback thread:
	pthread_create(&playbackThreadId, NULL, playbackThread, NULL);
	// Launch beat thread:
	pthread_create(&enqueueBeatThreadId, NULL, enqueueBeatThread, NULL);
}


// Client code must call AudioMixer_freeWaveFileData to free dynamically allocated data.
void AudioMixer_readWaveFileIntoMemory(char *fileName, wavedata_t *pSound) {
	assert(pSound);

	// The PCM data in a wave file starts after the header:
	const int PCM_DATA_OFFSET = 44;

	// Open the wave file
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		fprintf(stderr, "ERROR: Unable to open file %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	int sizeInBytes = ftell(file) - PCM_DATA_OFFSET;
	pSound->numSamples = sizeInBytes / SAMPLE_SIZE;

	// Search to the start of the data in the file
	fseek(file, PCM_DATA_OFFSET, SEEK_SET);

	// Allocate space to hold all PCM data
	pSound->pData = malloc(sizeInBytes);
	if (pSound->pData == 0) {
		fprintf(stderr, "ERROR: Unable to allocate %d bytes for file %s.\n",
				sizeInBytes, fileName);
		exit(EXIT_FAILURE);
	}

	// Read PCM data from wave file into memory
	int samplesRead = fread(pSound->pData, SAMPLE_SIZE, pSound->numSamples, file);
	if (samplesRead != pSound->numSamples) {
		fprintf(stderr, "ERROR: Unable to read %d samples from file %s (read %d).\n",
				pSound->numSamples, fileName, samplesRead);
		exit(EXIT_FAILURE);
	}
}

void AudioMixer_freeWaveFileData(wavedata_t *pSound) {
	pSound->numSamples = 0;
	free(pSound->pData);
	pSound->pData = NULL;
}

void AudioMixer_queueSound(wavedata_t *pSound) {
	// Ensure we are only being asked to play "good" sounds:
	assert(pSound->numSamples > 0);
	assert(pSound->pData);

	// Insert the sound by searching for an empty sound bite spot
	/*
	 * REVISIT: Implement this:
	 * 1. Since this may be called by other threads, and there is a thread
	 *    processing the soundBites[] array, we must ensure access is threadsafe.
	 * 2. Search through the soundBites[] array looking for a free slot.
	 * 3. If a free slot is found, place the new sound file into that slot.
	 *    Note: You are only copying a pointer, not the entire data of the wave file!
	 * 4. After searching through all slots, if no free slot is found then print
	 *    an error message to the console (and likely just return vs asserting/exiting
	 *    because the application most likely doesn't want to crash just for
	 *    not being able to play another wave file.
	 */

	int searchStart = freeSpaceCursor;
	int spaceCounter = 0;

	pthread_mutex_lock(&mutSoundBites);

	for ( ; freeSpaceCursor < MAX_SOUND_BITES; freeSpaceCursor++) {
		// search from space where the last pSound was placed

		if (soundBites[freeSpaceCursor].pSound == NULL) {
			// CASE: found, add pSound to slot and return
			soundBites[freeSpaceCursor].pSound = pSound;
			pthread_mutex_unlock(&mutSoundBites);

			freeSpaceCursor++;
			return;
		}

		spaceCounter++;
	}

	for (freeSpaceCursor = 0; freeSpaceCursor < searchStart; freeSpaceCursor++) {
		// wrap around to the start and search
		if (soundBites[freeSpaceCursor].pSound == NULL) {
			// CASE: found, add pSound to slot and return
			soundBites[freeSpaceCursor].pSound = pSound;
			pthread_mutex_unlock(&mutSoundBites);

			freeSpaceCursor++;
			return;
		}
		spaceCounter++;
	}
	pthread_mutex_unlock(&mutSoundBites);

	// CASE: no free space found, throw error
	printf("ERROR: soundBites contains %d active sounds, can't add new sound\n", spaceCounter);
	
}

void AudioMixer_playSound(enum drum drumSound) {
	switch (drumSound) {
		case HIGHHAT:
			AudioMixer_queueSound(&dHH);
			break;

		case BASS:
			AudioMixer_queueSound(&dBass);
			break;

		case SNARE:
			AudioMixer_queueSound(&dSnare);
			break;
		
		default:
			break;
	}
}


void AudioMixer_cleanup(void) {
	printf("Stopping audio...\n");

	// Stop the PCM generation thread
	stopping = true;
	pthread_join(playbackThreadId, NULL);

	// Shutdown the PCM output, allowing any pending sound to play out (drain)
	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	// Free playback buffer
	// (note that any wave files read into wavedata_t records must be freed
	//  in addition to this by calling AudioMixer_freeWaveFileData() on that struct.)
	free(playbackBuffer);
	playbackBuffer = NULL;

	printf("Done stopping audio...\n");
	fflush(stdout);
}


int AudioMixer_getVolume() {
	// Return the cached volume; good enough unless someone is changing
	// the volume through other means and the cached value is out of date.
	int temp;
	pthread_mutex_lock(&mutVolume);
	{
		temp = volume;
	}
	pthread_mutex_unlock(&mutVolume);

	return temp;
}

// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void AudioMixer_setVolume(int newVolume) {
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	
	if (newVolume < AUDIOMIXER_MIN_VOLUME || newVolume > AUDIOMIXER_MAX_VOLUME) {
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}

	pthread_mutex_lock(&mutVolume);

	volume = newVolume;

    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
	
	pthread_mutex_unlock(&mutVolume);
}


int AudioMixer_getBPM() {

	int temp;
	pthread_mutex_lock(&mutBPM);
	{
		temp = bpm;
	}
	pthread_mutex_unlock(&mutBPM);

	return temp;

}

void AudioMixer_setBPM(int newBPM) {
	if (newBPM < AUDIOMIXER_MIN_BPM || newBPM > AUDIOMIXER_MAX_BPM) {
		printf("ERROR: BPM must be between 40 and 300.\n");
		return;
	}
	
	pthread_mutex_lock(&mutBPM);
	{	
		bpm = newBPM;
	}
	pthread_mutex_unlock(&mutBPM);
}

// Fill the playbackBuffer array with new PCM values to output.
//    playbackBuffer: buffer to fill with new PCM data from sound bites.
//    size: the number of values to store into playbackBuffer
static void fillPlaybackBuffer(short *playbackBuffer, int size) {
	/*
	 * REVISIT: Implement this
	 * 1. Wipe the playbackBuffer to all 0's to clear any previous PCM data.
	 *    Hint: use memset()
	 * 2. Since this is called from a background thread, and soundBites[] array
	 *    may be used by any other thread, must synchronize this.
	 * 3. Loop through each slot in soundBites[], which are sounds that are either
	 *    waiting to be played, or partially already played:
	 *    - If the sound bite slot is unused, do nothing for this slot.
	 *    - Otherwise "add" this sound bite's data to the play-back buffer
	 *      (other sound bites needing to be played back will also add to the same data).
	 *      * Record that this portion of the sound bite has been played back by incrementing
	 *        the location inside the data where play-back currently is.
	 *      * If you have now played back the entire sample, free the slot in the
	 *        soundBites[] array.
	 *
	 * Notes on "adding" PCM samples:
	 * - PCM is stored as signed shorts (between SHRT_MIN and SHRT_MAX).
	 * - When adding values, ensure there is not an overflow. Any values which would
	 *   greater than SHRT_MAX should be clipped to SHRT_MAX; likewise for underflow.
	 * - Don't overflow any arrays!
	 * - Efficiency matters here! The compiler may do quite a bit for you, but it doesn't
	 *   hurt to keep it in mind. Here are some tips for efficiency and readability:
	 *   * If, for each pass of the loop which "adds" you need to change a value inside
	 *     a struct inside an array, it may be faster to first load the value into a local
	 *      variable, increment this variable as needed throughout the loop, and then write it
	 *     back into the struct inside the array after. For example:
	 *           int offset = myArray[someIdx].value;
	 *           for (int i =...; i < ...; i++) {
	 *               offset ++;
	 *           }
	 *           myArray[someIdx].value = offset;
	 *   * If you need a value in a number of places, try loading it into a local variable
	 *          int someNum = myArray[someIdx].value;
	 *          if (someNum < X || someNum > Y || someNum != Z) {
	 *              someNum = 42;
	 *          }
	 *          ... use someNum vs myArray[someIdx].value;
	 *
	 */

	
	// (1)
	memset(playbackBuffer, 0, playbackBufferSize * SAMPLE_SIZE);

	// (2), (3)
	int totalSamples = 0;
	int currentSample = 0;
	int sample = 0;
	int currentSuperPos = 0;
	playbackSound_t *pSoundBite = NULL;
	
	pthread_mutex_lock(&mutSoundBites);
	for (int i = 0; i < MAX_SOUND_BITES; i++) {
		pSoundBite = &soundBites[i];
		if (pSoundBite->pSound == NULL) {
			// CASE: soundBite is empty, skip to next
			continue;
		}

		totalSamples = pSoundBite->pSound->numSamples;
		currentSample = pSoundBite->location;

		for (int superPos = 0; superPos < playbackBufferSize; superPos++) {
			// add soundBite's sample to superpositions in playbackBuffer

			if (currentSample >= totalSamples) {
				// CASE: added last sample of current soundBite 
				//       to playbackBuffer, 'free' from soundBites[]
				pSoundBite->pSound = NULL;
				pSoundBite->location = 0;
				break;
			}
			
			currentSuperPos = playbackBuffer[superPos];
			sample = pSoundBite->pSound->pData[currentSample];

			if (currentSuperPos + sample > SHRT_MAX) {
				// CASE: overflow buffer, clamp to max
				playbackBuffer[superPos] = SHRT_MAX;

			} else if (currentSuperPos + sample < SHRT_MIN) {
				// CASE: underflow buffer, clamp to min
				playbackBuffer[superPos] = SHRT_MIN;

			} else {
				// CASE: no need to clamp
				playbackBuffer[superPos] += sample;
			}

			currentSample += 1;

		}

		if (pSoundBite->pSound != NULL) {
			// CASE: did not put all remaining samples into
			//       playback buffer - update location
			pSoundBite->location = currentSample;
		}


	}
	pthread_mutex_unlock(&mutSoundBites);

}

void* enqueueBeatThread(void* arg) {
	int beatCounter = 1; // counts half beats

	AudioMixer_readWaveFileIntoMemory(AUD_DRUM_BASS, &dBass);
	AudioMixer_readWaveFileIntoMemory(AUD_DRUM_HIGHHAT, &dHH);
	AudioMixer_readWaveFileIntoMemory(AUD_DRUM_SNARE, &dSnare);

	while (1) {	
		switch (AudioMixer_getBeat()) {
			case BEAT1:
				playBeat1(beatCounter);
				break;

			case BEAT2:
				playBeat2(beatCounter);
				break;
			
			default:
				break;
		}
		
		//Time For Half Beat [sec] = 60 [sec/min] / BPM / 2 [half-beats per beat]
		beatDelay.tv_nsec = (60.0f / AudioMixer_getBPM() / 2) * 1000000000; // quarter notes
		//printf("Waiting for %ld nanoseconds...\n", beatDelay.tv_nsec);
		
		nanosleep(&beatDelay, NULL); // wait for next beat
		
		if (beatCounter == 8) {
			beatCounter = 1;
		} else {
			beatCounter += 1;
		}

	}

	return NULL;

}

static void playBeat1(int beatCounter) {
	switch (beatCounter) {
		case 1:
		case 5:
			AudioMixer_queueSound(&dHH);
			AudioMixer_queueSound(&dBass);
			break;

		case 2:
		case 6:
			AudioMixer_queueSound(&dHH);
			break;

		case 3:
		case 7:
			AudioMixer_queueSound(&dHH);
			AudioMixer_queueSound(&dSnare);
			break;

		case 4:
		case 8:
			AudioMixer_queueSound(&dHH);
			break;
		
		default:
			break;
	}
}

static void playBeat2(int beatCounter) {

	switch (beatCounter) {
		case 1:
			AudioMixer_queueSound(&dBass);
		case 2:
			//AudioMixer_queueSound(&dHH);
			break;
		case 3:
			AudioMixer_queueSound(&dSnare);
			break;
		case 4:
			AudioMixer_queueSound(&dBass);
			break;
		case 5:
			AudioMixer_queueSound(&dHH);
			AudioMixer_queueSound(&dBass);
			break;
		case 6:
			AudioMixer_queueSound(&dBass);
			AudioMixer_queueSound(&dHH);
			break;
		case 7:
			AudioMixer_queueSound(&dHH);
			AudioMixer_queueSound(&dSnare);
			AudioMixer_queueSound(&dBass);
			break;
		case 8:
			AudioMixer_queueSound(&dHH);
			break;
		
		default:
			break;
	}

}

void AudioMixer_changeBeat(enum beat newBeat) {
	if (newBeat > 2) {
		return;
	}
	pthread_mutex_lock(&mutBeat);
	currentBeat = newBeat;
	pthread_mutex_unlock(&mutBeat);
}

enum beat AudioMixer_getBeat() {
	enum beat temp;
	pthread_mutex_lock(&mutBeat);
	temp = currentBeat;
	pthread_mutex_unlock(&mutBeat);
	return temp;
}

void* playbackThread(void* arg) {

	while (!stopping) {
		// Generate next block of audio
		fillPlaybackBuffer(playbackBuffer, playbackBufferSize);


		// Output the audio
		snd_pcm_sframes_t frames = snd_pcm_writei(handle,
				playbackBuffer, playbackBufferSize);

		// Check for (and handle) possible error conditions on output
		if (frames < 0) {
			fprintf(stderr, "AudioMixer: writei() returned %li\n", frames);
			frames = snd_pcm_recover(handle, frames, 1);
		}
		if (frames < 0) {
			fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n",
					frames);
			exit(EXIT_FAILURE);
		}
		if (frames > 0 && frames < playbackBufferSize) {
			printf("Short write (expected %li, wrote %li)\n",
					playbackBufferSize, frames);
		}
	}

	return NULL;
}
















