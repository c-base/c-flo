while true ; do
    rm -fR /home/pi/.config/chromium >/dev/null 2>&1
    rm -fR /home/pi/Downloads >/dev/null 2>&1
    sleep 2

    if [ -d /home/pi/chromium-defaults ]; then
        cp -a /home/pi/chromium-defaults /home/pi/.config/chromium
    fi

    sleep 2
    chromium-browser --kiosk --home-page "http://c-flo/infodisplay/?msgflo_role={{ ansible_hostname }}&{% for url in urls%}msgflo_urls={{ url }}&{% endfor %}" --disk-cache-dir=/dev/null --disk-cache-size=52428800
done