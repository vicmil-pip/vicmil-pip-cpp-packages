"""
Installs everything necessary for this package

On linux: install minimal clang compiler from google drive

On windows: install minimal mingw64 compiler from google drive
"""

import zipfile
import os
import pathlib
import platform
import sys
import importlib


def get_directory_path(__file__in, up_directories=0):
    return str(pathlib.Path(__file__in).parents[up_directories].resolve()).replace("\\", "/")


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


def create_python_virtual_environment(env_directory_path):
    # Setup a python virtual environmet
    os.makedirs(env_directory_path, exist_ok=True) # Ensure directory exists
    run_command(f'"{sys.executable}" -m venv "{env_directory_path}"')


def pip_install_package_in_virtual_environment(env_directory_path, package):
    if not os.path.exists(env_directory_path):
        print("Invalid path")
        raise Exception("Invalid path")
  
    my_os = platform.system()
    if my_os == "Windows":
        run_command(f'"{env_directory_path}/Scripts/pip" install {package}')
    else:
        run_command(f'"{env_directory_path}/bin/pip" install {package}')


def add_other_venv_to_sys_path(other_venv_path):
    def get_site_packages_path(venv_path):
        """Returns the site-packages path for a given virtual environment."""
        python_version = f"python{sys.version_info.major}.{sys.version_info.minor}"
        
        # Construct the expected site-packages path
        if os.name == "nt":  # Windows
            site_packages_path = os.path.join(venv_path, "Lib", "site-packages")
        else:  # macOS/Linux
            site_packages_path = os.path.join(venv_path, "lib", python_version, "site-packages")

        return site_packages_path if os.path.exists(site_packages_path) else None

    other_venv_path = get_site_packages_path(other_venv_path)
    if not os.path.exists(other_venv_path):
        raise Exception(f"Path does not exist! {other_venv_path}")

    if not other_venv_path in sys.path:
        sys.path.insert(0, other_venv_path)  # Add other venv's site-packages to sys.path


def try_import_pip_package(package_name):
    try:
        importlib.import_module(package_name)
        return True
    except ImportError:
        return False
    

def install_missing_requirements(requirements_file_path, other_venv_path):
    # Step1: Get which packages are missing
    if not os.path.exists(requirements_file_path):
        print(f"Requirements file not found: {requirements_file_path}")
        return
    
    missing_requirements = list()
    with open(requirements_file_path, "r") as f:
        for line in f:
            # Skip comments and empty lines
            if not line:
                continue

            line = line.split("#")[0]
            line = line.strip()
            
            if not line:
                continue

            # Extract just the package name for import checking
            package_name = line.split("==")[0].strip()
            if not try_import_pip_package(package_name=package_name):
                missing_requirements.append(line)


    if len(missing_requirements) == 0:
        return # No missing requirements
    

    # Step2: Create a python virtual environment
    create_python_virtual_environment(other_venv_path)


    # Step3: Install missing packages to the virtual environment
    for package in missing_requirements:
        pip_install_package_in_virtual_environment(
            env_directory_path=other_venv_path,
            package=package
        )

    # Step4: Add virtual enviroment to path
    add_other_venv_to_sys_path(other_venv_path=other_venv_path)


def install():
    # Download compiler from google drive
    # mingw64 for windows
    # clang for linux

    install_missing_requirements(
        requirements_file_path=get_directory_path(__file__) + "/requirements.txt",
        other_venv_path=get_directory_path(__file__) + "/venv"
    )

    tmp_zip_file_path = get_directory_path(__file__) + "drive_download_temp.zip"
    drive_url = None
    if platform.system() == "Windows":
        drive_url = "https://drive.google.com/file/d/1Tgn5f0IkBI7NklPYb79Ez8400WfOefad/view?usp=drive_link"
    if platform.system() == "Linux":
        drive_url = "https://drive.google.com/file/d/1hWY2N9nFp67SLVNej0mFcLzc7oLDa0er/view?usp=drive_link"

    download_file_from_google_drive(
        drive_url=drive_url,
        output_file=tmp_zip_file_path
    )

    if not os.path.exists(tmp_zip_file_path):
        raise Exception("Download failed!")

    # Extract file contents by unzipping the downloaded file
    unzip_file(
        zip_file_path=tmp_zip_file_path,
        destination_folder=get_directory_path(__file__),
        delete_zip=True
    )


def delete_file(file: str):
    if os.path.exists(file):
        os.remove(file)


def unzip_file(zip_file_path: str, destination_folder: str, delete_zip=False):
    with zipfile.ZipFile(zip_file_path, 'r') as zip_ref:
        zip_ref.extractall(destination_folder)

    if delete_zip:
        delete_file(zip_file_path)


def download_file_from_google_drive(drive_url: str, output_file: str):
    def _extract_id_from_url(url: str):
        url2 = url.split("drive.google.com/file/d/")[1]
        file_id = url2.split("/")[0]
        return file_id
    
    def _download_large_file_from_google_drive(id, destination):
        import gdown

        # Construct the direct URL
        url = f"https://drive.google.com/uc?id={id}"

        # Download the file
        gdown.download(url, destination, quiet=False)

    file_id = _extract_id_from_url(drive_url)
    _download_large_file_from_google_drive(file_id, output_file)


if __name__ == "__main__":
    install()