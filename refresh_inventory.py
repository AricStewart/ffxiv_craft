#!/usr/bin/python
import os
import re
import time
import dotenv
import requests

dotenv.load_dotenv()

SQEXID = os.getenv("SQEXID")
PASSWORD = os.getenv("PASSWORD")
CHARACTER_ID = os.getenv("CHARACTER_ID")
RETAINER_COUNT = os.getenv("RETAINER_COUNT")
RETAINER_ID = list()
for x in range(1, int(RETAINER_COUNT) + 1):
    RETAINER_ID.append(os.getenv("RETAINER_ID_" + str(x)))

session = requests.session()
login = session.get('https://na.finalfantasyxiv.com/lodestone/account/login/')
print login

val = re.findall('(oauthlogin.send.*)"', login.text)
val = val[0].split('"', 2)
url = 'https://secure.square-enix.com/oauth/oa/'+val[0]
stored = re.findall('_STORED_" value=(.*)"', login.text)[0]
stored = stored.strip('"');
payload = {'sqexid':SQEXID,
           'password':PASSWORD,
           '_STORED_' : stored,
           'wfp':1}
core = session.post(url, payload)
print core

part2 = re.findall('action="(.*)"', core.text)[0]
part2 = part2.replace("&amp;", "&")
hidden = re.findall('value="(.*)"', core.text)[0]
payload = {'cis_sessid': hidden, 'provision' : "", '_c' : "1"}
core2 = session.post(part2, payload)
print core2

csrf = re.findall('csrf_token" value=(.*)"', core2.text)[0]
csrf = csrf.strip('"');
url = 'https://na.finalfantasyxiv.com/lodestone/api/account/select_character/'
stamp = int(time.time() * 1000)
payload = {'csrf_token': csrf,
           'character_id': CHARACTER_ID,
           '__timestamp': stamp}
core3 = session.post(url, payload)
print core3

for entry in RETAINER_ID:
    index = str(RETAINER_ID.index(entry) + 1)

    core4 = session.get('https://na.finalfantasyxiv.com/lodestone/character/' +
                        CHARACTER_ID + '/retainer/' + entry + '/baggage/')
    print index, core4
    f = open('./inventory/inv'+ index + '.html', 'wb')
    b = bytearray(core4.text, 'utf8')
    f.write(b)
    f.close()
