#ifndef AUDIO_H
#define AUDIO_H

#include <linux/usb.h>
#include <sound/pcm.h>

#define USB_N_URBS 4
#define USB_N_PACKETS_PER_URB 8
#define USB_MAX_FRAMES_PER_PACKET 6

struct microbookii;

struct microbookii_urb {
	int index;

	struct microbookii *mbii;
	struct microbookii_substream *stream;

	/* BEGIN DO NOT SEPARATE */
	struct urb instance;
	struct usb_iso_packet_descriptor packets[USB_N_PACKETS_PER_URB];
	/* END DO NOT SEPARATE */
	
	int packet_size[USB_N_PACKETS_PER_URB]; /* size of packets for next submission */
	struct list_head ready_list;
};

struct microbookii_substream {
	struct snd_pcm_substream *instance;

	u8 state;
	bool active;
	snd_pcm_uframes_t dma_off; /* current position in alsa dma_area */
	snd_pcm_uframes_t period_off; /* current position in current period */

	struct microbookii_urb urbs[USB_N_URBS];
	struct list_head ready_playback_urbs;
	long unsigned int active_mask;
	
	unsigned int max_packet_size;
	struct microbookii_usb_packet_info {
		uint32_t packet_size[USB_N_PACKETS_PER_URB];
		int packets;
	} next_packet[USB_N_URBS];
	int next_packet_read_pos, next_packet_write_pos;

	spinlock_t lock;
	struct mutex mutex;
	wait_queue_head_t wait_queue;
	bool wait_cond;

        /** timestamps and delays **/
        bool trigger_tstamp_pending_update; /* update trigger_tstamp after initial calculation */
        
        int last_usb_frame;
        int last_delay;
};

struct microbookii_pcm {
	struct microbookii *mbii;

	struct snd_pcm *instance;
	struct snd_pcm_hardware pcm_playback_info;
	struct snd_pcm_hardware pcm_capture_info;

	struct microbookii_substream capture;
	struct microbookii_substream playback;
	
	bool panic; /* if set driver won't do anymore pcm on device */
};

int microbookii_init_audio(struct microbookii *mbii);
void microbookii_free_audio(struct microbookii *mbii);

#endif
