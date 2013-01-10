ffmpeg -i sad.mp3 -f u16le -acodec pcm_s16le -ar 44100 -ac 1  - |./filter
