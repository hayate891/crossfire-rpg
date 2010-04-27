# npc_dialog.py - Dialog helper class
#
# Copyright (C) 2007 David Delbecq
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#
# This is a simple script that makes use of CFDialog.py and that receives
# parameters from a JSON inside the event message. Alternatively, the JSON
# parameters, if >= 4096 characters, can be stored in a separate file.
# Use the classical script parameter to specify relative location of dialog.
#
# An example of a map file entry is:
#
# arch guildmaster
# name Sigmund
# msg
#
# endmsg
# x 11
# y 7
# resist_physical 100
# resist_magic 100
# weight 50000000
# friendly 1
# stand_still 1
# arch event_say
# name start/sigmund.msg
# title Python
# slaying /python/misc/npc_dialog.py
# end
# end
#
# An example of a JSON dialog similar to the one described in CFDialog.py is:
#
# {
#   "location" : "test_grandpa_01",
#   "rules": [
#   {
#     "match" : ["hello","hi"],
#     "pre" : [["hello","0"]],
#     "post" : [["hello","1"]],
#     "msg" : ["Hello, lad!","Hi, young fellow!","Howdy!"]
#   },
#   {
#     "match": ["hello","hi"],
#     "pre" :[["hello","1"]],
#     "post" :[["hello", "*"]],
#     "msg" : ["I've heard, you know, I'm not deaf *grmbl*"]
#   },
#   {
#     "match" : ["*"],
#     "pre" : [["hello","*"]],
#     "post" : [["hello", "*"]],
#     "msg" : ["What ?", "Huh ?", "What do you want ?"]
#   }
# ]}
#
# For detailed descriptions of the match, pre, post, and msg formats, see the
# ../CFDialog.py script.
#
# "match" is a list of keyword strings, and corresponds to what the player says
# that the dialog will respond to.
#
# In the above example, the first rule is applied if the player/character says
# "hello" or "hi" and if the "hello" flag is set to "0" (default).  When the
# rule is applied, the "hello" flag is then set to "1".
#
# "pre" is a list of preconditions that identifies flags that must be set to a
# particular value in order to trigger a response if a match is detected.
#
# "post" is a list of postconditions that specify flags that are to be set if a
# response is triggered.
#
# All of the rule values are lists, and must be enclosed by square braces, but
# pre and post are lists of lists, so the nested square braces ([[]]) are
# required except that using an empty list [] is the best way to indicate when
# the rule does not need to check preconditions or set postconditions.
#
# "msg" defines one or more responses that will be given if the rule triggers.
# When more than one "msg" value is set up, the NPC randomly selects which one
# to say each time the rule is applied.
#
# A relatively complex example of an npc_dialog.py dialog is given in the Gork
# treasure room quest.  See ../scorn/kar/gork.msg in particular as it
# demonstrates how multiple precondition flag values may be exploited to
# produce non-linear and variable-path conversations that are less likely to
# frustrate a player.  Refer also to ../scorn/kar/mork.msg to see how more
# than one dialog can reference the same dialog flags.

import Crossfire
import os
from CFDialog import DialogRule, Dialog
import cjson

npc = Crossfire.WhoAmI()
event = Crossfire.WhatIsEvent()
player = Crossfire.WhoIsActivator()
if (Crossfire.ScriptParameters() != None):
    filename = os.path.join(Crossfire.DataDirectory(),
                            Crossfire.MapDirectory(),
                            Crossfire.ScriptParameters())
    try:
        f = open(filename,'rb')
    except:
        Crossfire.Log(Crossfire.LogDebug, "Error loading NPC dialog %s" % filename)
        raise
    else:
        Crossfire.Log(Crossfire.LogDebug, "Loading NPC dialog %s" % filename)
        parameters = cjson.decode(f.read())
        f.close()
else:
    parameters = cjson.decode(event.Message)
location = parameters["location"];
speech = Dialog(player, npc, location)
index = 0;

for jsonRule in parameters["rules"]:
    rule = DialogRule(jsonRule["match"],
                      jsonRule["pre"],
                      jsonRule["msg"],
                      jsonRule["post"])
    speech.addRule(rule, index)
    index = index + 1

if speech.speak(Crossfire.WhatIsMessage()) == 0:
    Crossfire.SetReturnValue(1)
