#!/usr/bin/python
import time, re, pygame, serial, os
import mysql.connector as mariadb
from gtts import gTTS
from twython import TwythonStreamer
from termcolor import colored

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

print colored('-----------','yellow')
print colored('hello', 'red'), colored('world', 'blue')
print colored('-----------','yellow')

# Search terms - Deprecated - now using relational tables
#TERMS = ['@POTUS','@WhiteHouse']
#actionList  = (['1','@POTUS','@WhiteHouse','@PressSec','@realDonaldTrump','@FLOTUS'],
#               ['2','@VP','@DeptofDefense','@DHSgov','@CIA','@UN','@StateDept','@USTreasury','@real_sessions','@HHSGov','@USChamber'],
#               ['3','@SenateMajLdr','@SpeakerRyan','@GOPLeader','@GOP','@NancyPelosi'],
#               ['4','@FoxNews','@foxheadlines','@foxnewsalert','@seanhannity','@oreillyfactor','@foxandfriends','@LouDobbs'])
#actionColor = ['red','yellow','magenta','cyan']

# Twitter application authentication
APP_KEY = 'idd0G91xrCTttBBRiEDtxGEry'
APP_SECRET = '3e8ZhY1RmaWPHsc6mJgwMA7IxbgPB2J2WknDyebSVo9sbxHmiQ'
OAUTH_TOKEN = '49630765-tLYMPuHgeLdfNqsQHTVJXltdve3Xi6mqLpvkCotr5'
OAUTH_TOKEN_SECRET = 'BzuZA6SAIF9BIg4DAuwggTvnLQdTGNzE2mtOyA2PBrd1q'

# -------------------------------------------------------------------------------------------

# Setup callbacks from Twython Streamer
class BlinkyStreamer(TwythonStreamer):
        def on_success(self, data):
                if 'text' in data:
			tweet = data['text'].encode('utf-8')
			patrn1 = re.compile('^(RT )?(.*?@.*?:)\s(.*$)')
			twitt = re.search(patrn1,tweet)
			if twitt is not None:
				ser.write('h')		#  Blink LED Heartbeat
				rtwit = twitt.group(1)
				from1 = twitt.group(2)
				body1 = twitt.group(3)
				body1 = re.sub(r'http\S+','',body1)
				body1 = re.sub('$amp;','and',body1)
				body1 = re.sub('&amp;','and',body1)
				body1 = re.sub('"','',body1)
				body1 = re.sub('@','',body1)
				body1 = re.sub('#','',body1)
				aList = 0

				if xList.has_key(from1):
					aList = xList[from1]
					actionType = xList[from1]
					aCommand = xCommand[from1]
					aColor = xColor[from1]
					aSound = xSound[from1]
					aTrig = from1

				# Capture Tweet & Current Counter
				cntr = tlist.insTweet(from1,body1,aList)

				# Print Current Tweet
				reTwit = False
				if rtwit is not None:
					reTwit = True
					from1 = from1 + ' (' + str.strip(rtwit) + str(cntr) + ')'
				elif from1 != str.strip(from1):
					print colored('! NEW TWEET !','blue')
					reTwit = False
				print from1
				print ' ' + body1
				
				# Print List Match Results, Send Action, & Play mp3
				if aList != 0:
					print colored('!!! ACTION ' + str(aList) + ' !!! - triggered by ' + aTrig + "[" + aSound + "]", aColor)
					ser.write(aCommand)
					pygame.mixer.music.load("sounds/"+aSound)
					pygame.mixer.music.play()
				print '----------------------------------------------'

		if ser.inWaiting() > 0:
			btnChk = ser.read()
			if btnChk == '1':
				tlist.captureTweets(10)
				tlist.releaseTweets(10)
				tlist.clear()
			if btnChk == '2':
				tlist.retrieveTweets(10)

		if time.time() > (tlist.lastTimer + tlist.capDelay):
			tlist.captureTweets(50)
			tlist.clear()
			tlist.lastTimer = time.time()

	def on_error(self, status_code, data):
		print 'ERR:' + str(status_code)

# -------------------------------------------------------------------------

class topTweets:
#	Class to store top tweets by actor
	def __init__ (self):
		topTweets.tweetList = {}		# initialize dictionary
		topTweets.twitList = {}			# initialize tweet library
		topTweets.capDelay = 600		# default capture delay (sec)
		topTweets.lastTimer = time.time()

	def clear(self):
		print '** Tweet List Cleared **'
		print '------------------------'
		topTweets.tweetList = {}		# clear dictionary
		topTweets.twitList = {}			# clear tweet library

	def insTweet(self, fromr, bodyr, listr):
		if topTweets.tweetList.has_key(bodyr):
			topTweets.tweetList[bodyr][0] += 1
		else:
			topTweets.tweetList[bodyr] = [1,fromr,listr]
		return str(topTweets.tweetList[bodyr][0])

	def releaseTweets(self,count):
		i = 0
		os.system( 'amixer -q set PCM -- 100%' )
		print '!!! PLAYING LATEST TOP', str(count),'TWEETs !!!'
		for key, value in sorted(topTweets.tweetList.items(), key=lambda (k,v): (v,k), reverse = True):
			print "%s) %s[%s] %s %s" % (str(i+1),str(value[0]),str(value[2]),str(value[1]), str(key))
			if value[0] != 0:
#				tts = gTTS(text=key, lang='en')
#				tts.save('sounds/~temp.mp3')
#				pygame.mixer.music.load('sounds/~temp.mp3')
#				pygame.mixer.music.play()
				os.system( 'flite -t "' + key + '"' )
			i += 1
			if i >= count:
				break
		print '---------------------------------------------------'
		os.system( 'amixer -q set PCM -- 80%' )

	def captureTweets(self,count):
		mariadb_connection = mariadb.connect(user='Trublet', password='notsecret', database='Trubbles')
		os.system( 'amixer -q set PCM -- 100%' )
		i = 0
		print '!!! CAPTURING LATEST TOP', str(count),'TWEETs !!!'
		sqlstmt = 'INSERT INTO tweetStore (listr,fromr,bodyr,countr,createDate) VALUES '
		for key, value in sorted(topTweets.tweetList.items(), key=lambda (k,v): (v,k), reverse = True):
			if i != 0: 
				sqlstmt += ', '
			sqlstmt += '(%s,"%s","%s",%s,CURRENT_TIMESTAMP)' % (str(value[2]),str(value[1]),str(mariadb_connection.converter.escape(key)),str(value[0]))
			i += 1
			if i >= count:
				break
		if i != 0:
			sqlstmt = sqlstmt + ' ON DUPLICATE KEY UPDATE countr = countr + VALUES(countr), updateDate = CURRENT_TIMESTAMP;'
#			print "SQLSTMT = ",sqlstmt
			cursor = mariadb_connection.cursor(buffered=True)
			cursor.execute(sqlstmt)
			mariadb_connection.commit()
			print str(i) + ' Tweets Stored'
		else:
			print 'No Tweets to Store'
		print '---------------------------------------------------'
		mariadb_connection.close()

	def retrieveTweets(self,count):
		print '!!! PLAYING ALL-TIME TOP', str(count),'TWEETs !!!'
		i = 0
		mariadb_connection = mariadb.connect(user='Trublet', password='notsecret', database='Trubbles')
		cursor = mariadb_connection.cursor(buffered=True)
		sqlstmt = "SELECT countr, fromr, bodyr, listr FROM tweetStore ORDER BY 1 DESC LIMIT %s;" % count
#		print sqlstmt
		cursor.execute(sqlstmt)
		for rCountr, rFromr, rBodyr, rListr in cursor:
			print "%s) %s[%s] %s %s" % (str(i+1),str(rCountr),str(rListr), str(rFromr),str(rBodyr))
			if rCountr != 0:
				os.system( 'flite -t "' + rBodyr + '"' )
			i += 1
			if i >= count:
				break
		print '---------------------------------------------------'
		os.system( 'amixer -q set PCM -- 80%' )
		mariadb_connection.close()

# -------------------------------------------------------------------------------------------
# Setup / Initialize
# -------------------------------------------------------------------------------------------
pygame.mixer.init()
os.system( 'amixer -q set PCM -- 80%' )
tlist = topTweets()

# Connect to Database
mariadb_connection = mariadb.connect(user='Trublet', password='notsecret', database='Trubbles')
cursor = mariadb_connection.cursor(buffered=True)

# Capture Terms for Twitter
cursor.execute("SELECT topic from topics where active is true;")
array = cursor.fetchall()
TERMS = []
for row in array:
        TERMS.append(row[0].encode("utf-8"))

# Capture Lists, & Actors
cursor.execute("\
SELECT A.listid, CONCAT(B.actor,':'), A.command, A.textColor, \
       CASE WHEN B.soundfile IS NULL THEN A.soundfile ELSE B.soundfile END AS soundfile \
  FROM list_defaults A \
 INNER JOIN actors B \
    ON B.listid = A.listid \
 WHERE A.active is True;")

xList,xCommand,xColor,xSound = {},{},{},{}
for mList, mActor, mCommand, mColor, mSound in cursor:
#	print("ListID: {}, Actor: {}, Command: {}, Color: {}, Soundfile: {}").format (mList,mActor,mCommand,mColor,mSound)
	xList[mActor] = mList
	xCommand[mActor] = mCommand
	xColor[mActor] = mColor
	xSound[mActor] = mSound

mariadb_connection.close()

# Setup Serial IO to Arduino
ser = serial.Serial('/dev/ttyACM0',115200)

# Main Loop
running = True

while running:
	# Create Streamer
	try:
		stream = BlinkyStreamer(APP_KEY, APP_SECRET, OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
		stream.statuses.filter(track=TERMS)
	except KeyboardInterrupt:
		running = False
		print colored('\n-----------','yellow')
		print colored('Goodbye.','cyan')
		print colored('-----------','yellow')
	except Exception as e:
		print 'ERROR: ' + str(e)
		continue
	



