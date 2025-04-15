import platform
import pathlib
import zipfile
import os
import requests
import importlib
import sys


def get_compiler_path():
    if platform.system() == "Windows": # Windows
        return '"' + get_directory_path(__file__, 0) + "/emsdk/upstream/emscripten/em++.bat" + '"'
    else:
        return '"' + get_directory_path(__file__, 0) + "/emsdk/upstream/emscripten/em++" + '"'
    

def get_output_file_extension():
    return ".html"


def install():
    install_missing_requirements(
        requirements_file_path=get_directory_path(__file__) + "/requirements.txt",
        other_venv_path=get_directory_path(__file__) + "/venv"
    )

    emsdk_output_directory: str = get_directory_path(__file__) + "/emsdk"
    if platform.system() == "Linux":
        # Download emsdk from git
        emsdk_git_url = "https://github.com/emscripten-core/emsdk/archive/refs/heads/main.zip"
        tmp_zip = get_directory_path(__file__, 0) + "/emsdk_temp.zip"
        download_github_repo_as_zip(emsdk_git_url, tmp_zip)
        unzip_without_top_dir(tmp_zip, emsdk_output_directory, True)

        # Install emsdk
        emsdk_path = emsdk_output_directory + "/emsdk"
        run_command('chmod +x "' + emsdk_path + '"')
        run_command('"' + emsdk_path + '" install latest')
        run_command('"' + emsdk_path + '" activate latest')
    elif platform.system() == "Windows":
        tmp_zip = get_directory_path(__file__, 0) + "/emsdk_temp.zip"
        google_drive_path = "TODO"
        download_file_from_google_drive(google_drive_path, tmp_zip)
        unzip_without_top_dir(tmp_zip, emsdk_output_directory, True)

        old_name = emsdk_output_directory + "/emsdk-win"
        new_name = emsdk_output_directory + "emsdk"
        os.rename(old_name, new_name)


def unzip_without_top_dir(zip_file_path, destination_folder, delete_zip=False):
    with zipfile.ZipFile(zip_file_path, 'r') as zip_ref:
        # Get the list of file paths in the zip
        members = zip_ref.namelist()
        
        # Identify the top-level directory (assume first path element)
        top_level_dir = os.path.commonprefix(members).rstrip('/')
        
        for member in members:
            # Remove the top-level directory from the file path
            relative_path = os.path.relpath(member, top_level_dir)
            
            # Compute the final extraction path
            final_path = os.path.join(destination_folder, relative_path)

            if member.endswith('/'):  # Handle directories
                os.makedirs(final_path, exist_ok=True)
            else:  # Extract files
                os.makedirs(os.path.dirname(final_path), exist_ok=True)
                with zip_ref.open(member) as src, open(final_path, 'wb') as dst:
                    dst.write(src.read())

    if delete_zip:
        delete_file(zip_file_path)



def try_import_pip_package(package_name):
    try:
        importlib.import_module(package_name)
        return True
    except ImportError:
        return False



def pip_install_package_in_virtual_environment(env_directory_path, package):
    if not os.path.exists(env_directory_path):
        print("Invalid path")
        raise Exception("Invalid path")
  
    my_os = platform.system()
    if my_os == "Windows":
        run_command(f'"{env_directory_path}/Scripts/pip" install {package}')
    else:
        run_command(f'"{env_directory_path}/bin/pip" install {package}')


def create_python_virtual_environment(env_directory_path):
    # Setup a python virtual environmet
    os.makedirs(env_directory_path, exist_ok=True) # Ensure directory exists
    run_command(f'"{sys.executable}" -m venv "{env_directory_path}"')


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


    if len(missing_requirements) == 0:
        return # No missing requirements
    

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


def get_directory_path(__file__in, up_directories=0):
    return str(pathlib.Path(__file__in).parents[up_directories].resolve()).replace("\\", "/")


def delete_file(file: str):
    if os.path.exists(file):
        os.remove(file)


def unzip_file(zip_file_path: str, destination_folder: str, delete_zip=False):
    with zipfile.ZipFile(zip_file_path, 'r') as zip_ref:
        zip_ref.extractall(destination_folder)

    if delete_zip:
        delete_file(zip_file_path)


def download_github_repo_as_zip(zip_url: str, output_zip_file: str):
    """Downloads a GitHub repository as a ZIP file.
    
    Args:
        repo_url (str): The URL of the GitHub repository (e.g., "https://github.com/owner/repo").
        output_file (str): The name of the output ZIP file (e.g., "repo.zip").
    """
    try:
        response = requests.get(zip_url, stream=True)
        response.raise_for_status()  # Raise an error for bad responses
        
        with open(output_zip_file, "wb") as file:
            for chunk in response.iter_content(chunk_size=1024):
                file.write(chunk)
        
        print(f"Download complete: {output_zip_file}")
    except Exception as e:
        print(f"Error: {e}")


def download_file_from_google_drive(drive_url: str, output_file: str):
    def _extract_id_from_url(self, url: str):
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


def launch_html_page(html_file_path: str):
    import webbrowser
    """ Start the webbrowser if not already open and launch the html page

    Parameters
    ----------
        html_file_path (str): The path to the html file that should be shown in the browser

    Returns
    -------
        None
    """
    os.chdir(get_directory_path(html_file_path, 0))
    if not (os.path.exists(html_file_path)):
        print("html file does not exist!")
        return
    
    file_name: str = html_file_path.replace("\\", "/").rsplit("/", maxsplit=1)[-1]
    webbrowser.open("http://localhost:8000/" + file_name, new=0, autoraise=True)

    try:
        run_command("python3 -m http.server --bind localhost")
    except Exception as e:
        pass

    run_command("python -m http.server --bind localhost")


if __name__ == "__main__":
    install()