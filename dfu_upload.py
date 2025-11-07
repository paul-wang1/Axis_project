# dfu_upload.py
Import("env")
import os, subprocess

def custom_upload(source, target, env):
    bin_path = os.path.join(env['BUILD_DIR'], 'firmware.bin')
    # Run dfu-util but don't fail the build if it returns 74 after leave/reset
    subprocess.run(["dfu-util", "-a", "0", "-s", "0x08000000:leave", "-D", bin_path], check=False)
    print("dfu-util finished (ignoring final get_status).")

env.Replace(UPLOADCMD=custom_upload)