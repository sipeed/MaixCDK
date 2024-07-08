#
# @file from https://github.com/Neutree/c_cpp_project_framework
# @author neucrack
# @license MIT
#

import argparse
import os
import flask
from PIL import Image, ImageDraw
import netifaces as ni
import qrcode

parser = argparse.ArgumentParser(add_help=False, prog="deploy.py")
cmds = ["deploy"]

############################### Add option here #############################
# parser.add_argument("-p", "--port", default=8888, type=int, help="deploy server port")

#############################################################################

# use project_args created by SDK_PATH/tools/cmake/project.py, e.g. project_args.terminal

app = flask.Flask(__name__)
release_path = None

@app.route("/")
def index():
    if release_path:
        return flask.send_file(release_path)
    else:
        return flask.abort(404)


def serve(pkg_path, port = 8888):
    # serve deployment program
    global release_path
    release_path = pkg_path
    # show qr code of url
    local_ip = None
    local_port = int(port)
    ifs = ni.interfaces()
    ips = []
    for name in ifs:
        print(name)
        name_lower = name.lower()
        if name_lower.startswith("lo") or name_lower.startswith("docker") or name_lower.startswith("veth") or name_lower.startswith("vmware"):
            continue
        if ni.AF_INET not in ni.ifaddresses(name): # interface not get ip
            continue
        ip = ni.ifaddresses(name)[ni.AF_INET][0]['addr']
        print("-- fimd ip: {}".format(ip))
        if ip.startswith("192.168."):
            print(f"-- find local ip {name}: {ip}")
            ips.append(ip)
        elif ip.startswith("10."):
            print(f"-- find local ip {name}: {ip}")
            ips.append(ip)
    final_ip = None
    final_img = None
    os.makedirs("dist", exist_ok=True)
    for local_ip in ips:
        # show qr code
        url = "http://{}:{}".format(local_ip, local_port)
        print("Scan the QR code to download the program:")
        # generate qr code
        img = qrcode.make(url)
        # show img
        tmp_qr_path = os.path.join("dist", f"tmp_qr_{local_ip}.png")
        print("\n----------------------------------")
        print(f"   QR code in !! {tmp_qr_path} !!")
        print(os.path.abspath(tmp_qr_path))
        print("----------------------------------\n")
        img.save(tmp_qr_path)
        img = Image.open(tmp_qr_path)
        img_big = Image.new("RGB", (img.width, img.height + 32), (255, 255, 255))
        img_big.paste(img, (0, 0))
        # add url text
        draw = ImageDraw.Draw(img_big)
        draw.text((64, img.height - 16), url, (0, 0, 0))
        img_big.save(tmp_qr_path)
        if not final_img:
            final_img = img_big
            final_ip = local_ip
        if local_ip.startswith("192.168."):
            final_ip = local_ip
            final_img = img_big
    final_img.save(os.path.join("dist", "tmp_qr_all.png"))
    try:
        final_img.show(title="Scan the QR code to install")
    except Exception:
        print("")
        print("------------------------")
        print("[Error] show image failed, you can remain see QR code at dist directory")
        print("------------------------")
    # url = "http://{}:{}".format(, vars["project_args"].port)
    app.run(host="0.0.0.0", port = local_port)


