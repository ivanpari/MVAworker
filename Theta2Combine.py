#!/usr/bin/env python2

from __future__ import print_function
import ROOT
import sys

fi = ROOT.TFile.Open(sys.argv[1], "read")
fo = ROOT.TFile(sys.argv[2], "RECREATE")

fi.cd()
keyList = ROOT.gDirectory.GetListOfKeys()

fo.cd()
for key in keyList:
    h = key.ReadObj()

    oName = h.GetName()
    h.SetName(h.GetName().replace("__plus", "Up"))
    h.SetName(h.GetName().replace("__minus", "Down"))
    h.SetName(h.GetName().replace("DATA", "data_obs"))

    if oName != h.GetName():
        print(oName, "-->", h.GetName())
    else:
        print(oName)

    h.Write()

fi.Close()
fo.Close()
