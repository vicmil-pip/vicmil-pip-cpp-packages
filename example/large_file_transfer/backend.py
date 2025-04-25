# app.py
from flask import Flask, render_template
from flask_socketio import SocketIO, emit
from flask_cors import CORS
import base64
import os
import pathlib

app = Flask(__name__)
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*")

@app.route('/')
def index():
    return render_template('index.html')

def get_directory_path(__file__in, up_directories=0):
    return str(pathlib.Path(__file__in).parents[up_directories].resolve()).replace("\\", "/")


@socketio.on('connect')
def handle_connect():
    print("Client connected")


@socketio.on('start_download')
def send_file_chunks(data):
    filename = data.get('filename')

    filepath = get_directory_path(__file__) + "/bigfile.txt"  # safer with fixed directory

    if not os.path.exists(filepath):
        emit('download_error', {'message': 'File not found'})
        return

    chunk_size = 64 * 1024  # 64 KB

    with open(filepath, 'rb') as f:
        index = 0
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            base64_chunk = base64.b64encode(chunk).decode('utf-8')
            emit('file_chunk', {
                'index': index,
                'chunk': base64_chunk,
                'filename': filename
            })
            index += 1

    emit('download_complete', {'filename': filename})

if __name__ == '__main__':
    socketio.run(app, debug=True, port=5050)
