# -*- coding: utf-8 -*-

# Author: Tuzeeky
# Date  : 12/17/2017

# This script imports translated XML that are exported 
# by GJDSEC.exe to table.data.

# This script was written under Python 3.6.2
# Put this script under the same directory with data.table and GJDSEC.exe
# Usage: python ConvertAndImport.py <XMLFilename>

import os
import sys
import json
from xml.etree.ElementTree import Element, SubElement, tostring, ElementTree, parse

def main(argv):
    if len(sys.argv) <= 1:
        print('Please specify the TXT/XML file name!!\n')
        print('Usage: python ConvertAndImport.py <XMLFilename>\n')
        return

    sourceFilename = sys.argv[1]
    tableFilename = 'data.table'

    srcXML = parse(sourceFilename)
    srcXMLRoot = srcXML.getroot()
    tableRoot = srcXMLRoot.find('Select')  
    tableName = tableRoot.get('From')

    targetXMLRoot = Element('Request')
    tree = ElementTree(targetXMLRoot)

    for item in tableRoot:
        childrenCount = len(item.getchildren())
        idColumn = item[0].get('Column')
        updateNodeAttrib = {u'Table':tableName, u'Where': idColumn + '= \'' + item[0].text + '\''}
        updateNode = SubElement(targetXMLRoot, u'Update', updateNodeAttrib)

        for i in range(1, childrenCount):
            nodeAttrib = {u'Column':item[i].get('Column')}
            node = SubElement(updateNode, u'Value', nodeAttrib)
            node.text = item[i].text

    outCMD = u'<?xml version="1.0" encoding="utf-8" ?>'
    outXML = tostring(targetXMLRoot, encoding='utf-8', method='xml')

    outCMD += outXML.decode('utf-8')
    outCMD = json.dumps(outCMD)

    os.system('GJDSEC.exe -f ' + tableFilename + ' -s ' + outCMD) # Invoke GJDSEC.exe to import data from XML
    print ('\n\nFinished importing ' + sourceFilename)

    return

if __name__ == "__main__":
    main(sys.argv)
