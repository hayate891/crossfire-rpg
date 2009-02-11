# Script for say event of Imperial Bank Tellers
#
# Copyright (C) 2002 Joris Bontje
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
# The author can be reached via e-mail at jbontje@suespammers.org
#
# Updated to use new path functions in CFPython and broken and
# modified a bit by -Todd Mitchell


import Crossfire

import string
import random
import CFBank
import CFItemBroker

activator = Crossfire.WhoIsActivator()
activatorname = activator.Name
whoami = Crossfire.WhoAmI()
x = activator.X
y = activator.Y


#EASILY SETTABLE PARAMETERS
service_charge = 5    # service charges for transactions as a percent
exchange_rate = 10000 # exchange rate of imperial to silver (value 1)
bankdatabase = "ImperialBank_DB"

fees = (service_charge/100.0)+1
bank = CFBank.CFBank(bankdatabase)

text = Crossfire.WhatIsMessage().split()
thanks_message = [ \
	'Thank you for banking the Imperial Way.', \
	'Thank you for banking the Imperial Way.', \
	'Thank you, please come again.', \
	'Thank you, please come again.', \
	'Thank you for your patronage.', \
	'Thank you for your patronage.', \
	'Thank you, have a nice day.', \
	'Thank you, have a nice day.', \
	'Thank you. "Service" is our middle name.', \
	'Thank you. "Service" is our middle name.', \
	'Thank you. Hows about a big slobbery kiss?' ]


if text[0] == 'help' or text[0] == 'yes':
	message ='You can:\n-deposit,-withdraw,-balance,-exchange \
\nAll transactions are in imperial notes\n(1 : 1000 gold coins). \
\nA service charge of %d percent will be placed on all deposits.' \
		%(service_charge)


elif text[0] == 'deposit':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "deposit <amount in imperials>"'
		elif amount > 10000:
			message = 'Sorry, we do not accept more than 10000 imperials for one deposit.'
		elif activator.PayAmount(int(amount*exchange_rate*fees)):
			bank.deposit(activatorname, amount)
			message = '%d platinum received, %d imperials deposited to bank account. %s' \
			%((amount*(exchange_rate/50))*fees, amount, random.choice(thanks_message))
		else:
			message = 'You would need %d gold'%((amount*(exchange_rate/10))*fees)
	else:
		message = 'Usage "deposit <amount in imperials>"'


elif text[0] == 'withdraw':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "withdraw <amount in imperials>"'
		elif amount > 10000:
			message = 'Sorry, we do not accept more than 10000 imperials for one withdraw.'
		elif bank.withdraw(activatorname, amount):
			message = '%d imperials withdrawn from bank account. %s' \
			%(amount, random.choice(thanks_message))
			id = activator.Map.CreateObject('imperial', x, y)
			CFItemBroker.Item(id).add(amount)
			activator.Take(id)
		else:
			message = 'Not enough imperials on your account'
	else:
		message = 'Usage "withdraw <amount in imperials>"'


elif text[0] == 'exchange':
	if len(text) == 2:
		amount = int(text[1])
		if amount <= 0:
			message = 'Usage "exchange <amount>" (imperials to platinum coins)'
		elif amount > 10000:
			message = 'Sorry, we do not exchange more than 10000 imperials all at once.'
		else:
			inv = activator.CheckInventory('imperial')
			if inv:
				pay = CFItemBroker.Item(inv).subtract(amount)
				if pay:

					# Drop the coins on the floor, then try
					# to pick them up. This effectively
					# prevents the player from carrying too
					# many coins.
					id = activator.Map.CreateObject('platinum coin', x, y)
					CFItemBroker.Item(id).add(amount*(exchange_rate/50))
					activator.Take(id)

					message = random.choice(thanks_message)
				else:
					message = 'Sorry, you do not have %d imperials'%(amount)
			else:
				message = 'Sorry, you do not have any imperials'
	else:
		message = 'Usage "exchange <amount>" (imperials to platinum coins)'


elif text[0] == 'balance':
	balance = bank.getbalance(activatorname)
	if balance == 1:
		message = 'Amount in bank: 1 imperial note'
	elif balance:
		message = 'Amount in bank: %d imperial notes'%(balance)
	else:
		message = 'Sorry, you have no balance.'


else:
	message = 'Do you need help?'

whoami.Say(message)
Crossfire.SetReturnValue(1)
