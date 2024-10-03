#!/usr/bin/env python3

""" A """

import csv
import json

if __name__ == "__main__":
    INVENTORY = None
    with open('current.json', encoding='utf8') as jsonfile:
        INVENTORY = json.load(jsonfile)
    REPORT = []
    with open('tier.csv', encoding='utf8') as csvfile:
        reader = csv.reader(csvfile)
        for idx, row in enumerate(reader):
            if idx == 0:
                data = [ row[0], row[3],  row[8], 'sale', row[15] ]
            else:
                if row[0] in INVENTORY['items']:
                    continue
                if int(row[17]) <= 0 or int(row[15]) <= 0 or int(row[8]) <= 0:
                    continue
                if row[7]== '':
                    sale = '{:,}'.format(int(row[6])) + ' gil'
                else:
                    sale = '{:,}'.format(int(row[7])) + ' gil'
                cost = '{:,}'.format(int(row[3])) + ' gil'
                sale_count = '{:,}'.format(int(row[8])) + ' sales'
                profit = '{:,}'.format(int(row[15])) + ' gil'
                data = [' ' + row[0], ' ' + cost,
                        ' ' + sale_count, ' ' + sale, ' ' + profit]
            REPORT.append(data)

    with open('tier_filtered.csv', 'w', encoding='utf8') as f:
        writer = csv.writer(f)
        writer.writerows(REPORT)
