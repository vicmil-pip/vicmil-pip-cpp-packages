import pathlib
import os
import platform
import importlib
import sys
import time
import string
import random

def get_directory_path(__file__in, up_directories=0):
    return str(pathlib.Path(__file__in).parents[up_directories].resolve()).replace("\\", "/")


def _python_virtual_environment(env_directory_path):
    # Setup a python virtual environmet
    os.makedirs(env_directory_path, exist_ok=True) # Ensure directory exists
    run_command(f'"{sys.executable}" -m venv "{env_directory_path}"')


def _pip_install_packages_in_virtual_environment(env_directory_path, packages):
    if not os.path.exists(env_directory_path):
        print("Invalid path")
        raise Exception("Invalid path")

    my_os = platform.system()
    for package in packages:
        if my_os == "Windows":
            os.system(f'powershell; &"{env_directory_path}/Scripts/pip" install {package}')
        else:
            os.system(f'"{env_directory_path}/bin/pip" install {package}')


def _get_site_packages_path(venv_path):
    """Returns the site-packages path for a given virtual environment."""
    python_version = f"python{sys.version_info.major}.{sys.version_info.minor}"
    
    # Construct the expected site-packages path
    if os.name == "nt":  # Windows
        site_packages_path = os.path.join(venv_path, "Lib", "site-packages")
    else:  # macOS/Linux
        site_packages_path = os.path.join(venv_path, "lib", python_version, "site-packages")

    return site_packages_path if os.path.exists(site_packages_path) else None


def _get_python_path(venv_path):
    if platform.system() == "Windows":
        return venv_path + "/Scripts/python"
    else:
        return venv_path + "/bin/python"


def _use_other_venv_if_missing(package_name, other_venv_path, silent=False):
    try:
        importlib.import_module(package_name)
        if not silent:
            print(f"{package_name} is already installed in the current environment.")
    except ImportError:
        print(f"{package_name} not found. Using the package from the other environment.")
        other_venv_path = _get_site_packages_path(other_venv_path)
        if not other_venv_path or not os.path.exists(other_venv_path):
            print(f"Error: site-packages directory not found at {other_venv_path}")
            return
        else:
            print(f"Using site-packages directory found at {other_venv_path}")
        if not other_venv_path in sys.path:
            sys.path.append(other_venv_path)  # Add other venv's site-packages to sys.path


def run_command(command: str) -> None:
    """Run a command in the terminal"""
    platform_name = platform.system()
    if platform_name == "Windows": # Windows
        print("running command: ", f'powershell; &{command}')
        if command[0] != '"':
            os.system(f'powershell; {command}')
        else:
            os.system(f'powershell; &{command}')
    else:
        print("running command: ", f'{command}')
        os.system(command)


def go_to_url(url: str):
    # Opens the webbrowser with the provided url
    import webbrowser
    webbrowser.open(url, new=0, autoraise=True)


def generate_large_text_file(filename='bigfile.txt', size_in_mb=1024):
    if os.path.exists(filename):
        print(f"{filename} already exists.")
        return

    chunk_size = 1024 * 1024  # 1MB
    chars = string.ascii_letters + string.digits + ' \n'
    
    with open(filename, 'w') as f:
        for _ in range(size_in_mb):
            text = ''.join(random.choices(chars, k=chunk_size))
            f.write(text)

    print(f"{filename} generated with {size_in_mb} MB of random text.")


if __name__ == "__main__":
    env_dir = get_directory_path(__file__, 1) + "/venv"
    _python_virtual_environment(env_dir)
    _pip_install_packages_in_virtual_environment(env_directory_path=env_dir, packages=["flask", "Flask-SocketIO"])
    venv_python_path = _get_python_path(env_dir)
    go_to_url("http://127.0.0.1:5050")
    generate_large_text_file(filename=get_directory_path(__file__) + "/bigfile.txt", size_in_mb=100) # Generate a large file to upload
    run_command(f'"{venv_python_path}" "{get_directory_path(__file__)}/backend.py"')
    while True:
        time.sleep(1)

