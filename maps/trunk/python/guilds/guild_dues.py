#Script for paying Guild Dues
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
# author:Avion temitchell@sourceforge.net

import Crossfire
import CFGuilds
import CFItemBroker
import random
import string
gbfile='/cftmp/guildbalance.txt' #location of the balance of dues paid, where it should go depends on your distribution, uses an absolute path.
x=6
y=7
x1=31
x2=32
x3=33
x4=34
x5=35
y1=9
y2=10
y3=11
y4=12
y5=13
activator=Crossfire.WhoIsActivator()
activatorname=activator.Name
mymap = activator.Map
whoami=Crossfire.WhoAmI()
print whoami.Name
if whoami.Name=='Jack':
 remarklist = ['Excellent','Thank You','Thank You','Thank You', 'Thank You', 'Great', 'OK', 'Wonderful', 'Swell', 'Dude', 'Big Spender']
 exclaimlist = ['Hey','Hey','Hey','Hey', 'Now just a minute', 'AHEM', 'OK...Wait a minute', 'Look chowderhead']
 buddylist =   ['buddy','buddy','buddy','buddy','pal','friend','friend','friend','friend','dude','chum', 'sweetie']

 guildname=Crossfire.ScriptParameters() # 6 is say event
 text = Crossfire.WhatIsMessage().split()

 if (guildname):
     guild = CFGuilds.CFGuild(guildname)
     cointype = "jadecoin" #What type of token are we using for guild dues?
     object = activator.CheckInventory(cointype)

     if text[0] == 'help' or text[0] == 'yes':
         message='Let me know how many jade coins you want to pay.  Say pay <amount>'

     elif text[0] == 'pay':
         if len(text)==2:
             cost = int(text[1])
             if (object):
                 pay = CFItemBroker.Item(object).subtract(cost)
                 if (pay):
                     guild.pay_dues(activatorname,cost)
                     message = "%s, %d %s paid to the guild." %(random.choice(remarklist),cost, cointype)
                     guildbalance = open(str(gbfile), 'r')
                     guildbalance2 = guildbalance.read()
                     guildbalance1 = int(guildbalance2) + int(cost)
                     guildbalance.close()
                     guildbalance3 = open(str(gbfile), 'w')
                     guildbalance4 = str(guildbalance1)
                     guildbalance3.write(guildbalance4)
                     print guildbalance4

                 else:
                     if cost > 1:
                        message ="%s, you don't have %d %ss." %(random.choice(exclaimlist),cost,cointype)
                     else:
                         message ="You don't have any %s %s." %(cointype,random.choice(buddylist))
             else:
                message = "Come back when you got the %ss %s." %(cointype,random.choice(buddylist))
         else:
             message = "How much ya wanna pay %s?" %(random.choice(buddylist))
     else:
         message = "Howdy %s, paying some guild dues today?" %(random.choice(buddylist))
     whoami.Say(message)
 else:
     activator.Write('Guildname Error, please notify a DM')
else:
  guildbalance=open(str(gbfile), 'r')
  balance=guildbalance.read()
  x = activator.X
  y = activator.Y
  amount=int(balance)

  if amount <= 0:
      message = 'No dues have been paid.'
  else:
      message = '%d dues withdrawn.' % amount
      id = activator.Map.CreateObject('jadecoin', x, y)
      CFItemBroker.Item(id).add(amount)
      activator.Take(id)

  guildbalance.close()
  guildbalance3= open(str(gbfile), 'w')
  guildbalance3.write('0')
  guildbalance3.close()
