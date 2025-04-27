import sys
from pydub import AudioSegment
import io
import protocol

def convert_audio_to_wav_pcm(file_path):
    """
    Convert any audio file to mono 16-bit 44100Hz WAV and return as BytesIO.

    Args:
        file_path (str): path to input audio file

    Returns:
        BytesIO: WAV audio as a file-like object
    """
    # Load the input audio from path
    audio = AudioSegment.from_file(file_path)

    # Convert to mono, 16-bit, 44100 Hz
    audio = audio.set_channels(1).set_frame_rate(44100).set_sample_width(2)

    # Export into a BytesIO buffer
    buffer = io.BytesIO()
    audio.export(buffer, format="wav")
    buffer.seek(0)

    return buffer

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python convert.py <port> <input_file> <remote_name>")
        sys.exit(1)

    port = sys.argv[1]
    input_path = sys.argv[2]
    remote_name = sys.argv[3]

    device = protocol.ProtocolClient(port)

    wav_buffer = convert_audio_to_wav_pcm(input_path)

    size = wav_buffer.getbuffer().nbytes

    def update_progress(transferred, speed):
        print(f"Transferred: {transferred/size*100:.2f} %, Speed: {speed:.2f} KB/s", end="\r")

    device.push(wav_buffer, size, remote_name, update_progress)
