# while true ; do
    rm -fR /home/{{ display_user }}/.config/chromium >/dev/null 2>&1
    rm -fR /home/{{ display_user }}/Downloads >/dev/null 2>&1
    sleep 2

    if [ -d /home/{{ display_user }}/chromium-defaults ]; then
        cp -a /home/{{ display_user }}/chromium-defaults /home/pi/.config/chromium
    fi

    sleep 2
    chromium-browser --kiosk --no-first-run \
        --home-page "http://c-flo/infodisplay/?msgflo_role={{ ansible_hostname }}&{% for url in urls%}msgflo_urls={{ url|urlencode }}&{% endfor %}" \
        --disk-cache-dir=/dev/null --disk-cache-size=52428800
# done