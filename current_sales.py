#!/usr/local/bin/python3.11
""" A """

import time
import json
import os
import sys

from dotenv import load_dotenv

from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.options import Options
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium_recaptcha_solver import RecaptchaSolver

class SQEX:
    ''' A '''

    def __init__(self):
        self.sqexid = os.environ['SQEXID']
        self.password = os.environ['PASSWORD']
        self.character_id = os.environ['CHARACTER_ID']
        self.retainer_count = os.environ['RETAINER_COUNT']
        self.retainer_id = ( os.environ['RETAINER_ID_1'],
                             os.environ['RETAINER_ID_2'] )

        self.options = Options()
        self.driver = webdriver.Firefox(options=self.options)
        self.url = 'https://na.finalfantasyxiv.com/lodestone/account/login/'

    def start(self):
        '''A'''
        self.driver.get(self.url)
        element = self.driver.find_element(By.ID, 'sqexid')
        element.send_keys(self.sqexid)
        element = self.driver.find_element(By.ID, 'password')
        element.send_keys(self.password)
        time.sleep(1)


        #print("Solve reCaptcha")
        #solver = RecaptchaSolver(driver=self.driver)
        #recaptcha_iframe = self.driver.find_element(By.XPATH, '//iframe[@title="reCAPTCHA"]')
        #solver.click_recaptcha_v2(iframe=recaptcha_iframe)
        #time.sleep(5)
        #print("Going on...")

        input("Bypass reCaptura...")
        #while True:
        #    try :
        #        box = self.driver.find_element(By.CLASS_NAME, 'recaptcha-checkbox')
        #        print(box)
        #        value = box.get_attribute('aria-checked').lower()
        #        print(value)
        #        if box.get_attribute('aria-checked').lower() == 'true':
        #            break
        #    except Exception as e:
        #        print(e)

        self.driver.find_element(By.ID, 'view-loginArea').click()

        wait = WebDriverWait(self.driver, 10)
        element = wait.until(
                EC.element_to_be_clickable((By.CLASS_NAME,
                                            'osano-cm-accept-all')))
        element.click()
        self.driver.find_element(By.CLASS_NAME, 'bt_character_login').click()
        time.sleep(1)
        items=[]
        for entry in self.retainer_id:
            url = 'https://na.finalfantasyxiv.com/lodestone/character/' + \
                    self.character_id + '/retainer/' + entry + '/'
            self.driver.get(url)
            element = wait.until(
                    EC.element_to_be_clickable((By.PARTIAL_LINK_TEXT,
                                                'Sale Inventory')))
            element.click()
            lst = self.driver.find_element(By.NAME, 'tab__market-list')
            elements = lst.find_elements(By.CLASS_NAME, 'item-list__name')
            for elm in elements:
                full_text = elm.text
                blob = elm.find_element(By.TAG_NAME, 'span').text
                item_text = full_text.replace(blob, '')
                items.append(item_text)
        return list(set(items))

    def close(self):
        ''' A '''
        self.driver.close()


if __name__ == "__main__":
    load_dotenv()
    webscraper = SQEX()
    dct = {
        "items" : webscraper.start()
    }
    webscraper.close()
    with open('current.json', 'w', encoding='utf8') as f:
        json.dump(dct, f)
    print(dct)
    print(len(dct['items']))
