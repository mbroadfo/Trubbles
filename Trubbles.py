import time, re, pygame, serial, os
from twython import TwythonStreamer
from termcolor import colored
 
print colored('-----------','yellow')
print colored('hello', 'red'), colored('world', 'blue')
print colored('-----------','yellow')

# Search terms
TERMS = ['@POTUS','@WhiteHouse']
actionList  = (['1','@POTUS','@WhiteHouse','@PressSec','@realDonaldTrump','@FLOTUS'],
               ['2','@VP','@DeptofDefense','@DHSgov','@CIA','@UN','@StateDept','@USTreasury','@real_sessions','@HHSGov','@USChamber'],
               ['3','@SenateMajLdr','@SpeakerRyan','@GOPLeader','@GOP','@NancyPelosi'],
               ['4','@FoxNews','@foxheadlines','@foxnewsalert','@seanhannity','@oreillyfactor','@foxandfriends','@LouDobbs'])
actionColor = ['red','yellow','magenta','cyan']

# Twitter application authentication
APP_KEY = 'idd0G91xrCTttBBRiEDtxGEry'
APP_SECRET = '3e8ZhY1RmaWPHsc6mJgwMA7IxbgPB2J2WknDyebSVo9sbxHmiQ'
OAUTH_TOKEN = '49630765-tLYMPuHgeLdfNqsQHTVJXltdve3Xi6mqLpvkCotr5'
OAUTH_TOKEN_SECRET = 'BzuZA6SAIF9BIg4DAuwggTvnLQdTGNzE2mtOyA2PBrd1q'

os.system( 'amixer -q set PCM -- 80%' )

# -------------------------------------------------------------------------------------------

# Setup callbacks from Twython Streamer
class BlinkyStreamer(TwythonStreamer):
        def on_success(self, data):
                if 'text' in data:
			tweet = data['text'].encode('utf-8')
			patrn1 = re.compile('^(RT )?(.*?@.*?:)\s(.*$)')
			twitt = re.search(patrn1,tweet)
			if twitt is not None:
				ser.write('h')		#  Blink LED
				rtwit = twitt.group(1)
				from1 = twitt.group(2)
				body1 = twitt.group(3)
				body1 = re.sub(r'http\S+','',body1)
				body1 = re.sub('$amp;','&',body1)
				aList = 0

				for actions in actionList:
					n = 0
					for actor in actions:
						if n == 0:
							actionType = actor
						else:
							chk = actor + ':'
							patrn2 = re.compile(chk)
							match2 = re.search(patrn2,from1)
							if match2 is not None:
								aList = actionType
								aTrig = actor
								break
						n += 1

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
				print 'FROM: ' + from1
				print 'BODY: ' + body1
				
				# Print List Match Results, Send Action, & Play mp3
				if aList != 0:
					print colored('!!! ACTION ' + str(aList) + ' !!! - triggered by ' + aTrig,actionColor[int(aList)-1])
					if aList == '1':
						ser.write('abcdefgh')
						if reTwit == False: pygame.mixer.music.load("sounds/hail_to_the_chief.mp3")
						else:               pygame.mixer.music.load("sounds/hail.mp3")
					elif aList == '2':
						ser.write('abcd')
						if reTwit == False: pygame.mixer.music.load("sounds/helloo.mp3")
						else:               pygame.mixer.music.load("sounds/kittycry.mp3")
					elif aList == '3':
						ser.write('bcf')
						if reTwit == False: pygame.mixer.music.load("sounds/Baby+Giggle+1.mp3")
						else:               pygame.mixer.music.load("sounds/Burp+2.mp3")
					elif aList == '4':
						ser.write('d')
						if reTwit == False: pygame.mixer.music.load("sounds/cauldronbubbles.mp3")
						else:               pygame.mixer.music.load("sounds/baboon2.mp3")
					pygame.mixer.music.play()
				print '----------------------------------------------'

		if ser.inWaiting() > 0:
			btnChk = ser.read()
			if btnChk == '1':
				tlist.releaseTweets(10)
			if btnChk == '2':
				tlist.clear()
		
	def on_error(self, status_code, data):
		print 'ERR:' + str(status_code)

# -------------------------------------------------------------------------

class topTweets:
#	Class to store top tweets by actor
	def __init__ (self):
		topTweets.tweetList = {}		# initialize dictionary
		topTweets.twitList = {}			# initialize tweet library

	def clear(self):
		print '** Tweet List Cleared **'
		print '------------------------'
		topTweets.tweetList = {}		# clear dictionary
		topTweets.twitList = {}			# clear tweet library

	def insTweet(self, fromer, bodyr, listr):
		tweetr = fromer + ':' + bodyr
		if topTweets.tweetList.has_key(tweetr):
			topTweets.tweetList[tweetr][0] += 1
		else:
			topTweets.tweetList[tweetr] = [1,listr]
		return str(topTweets.tweetList[tweetr][0])

	def releaseTweets(self,count):
		i = 0
		os.system( 'amixer -q set PCM -- 100%' )
		print '!!! TOP ', str(count),' TWEETs !!!'
		for key, value in sorted(topTweets.tweetList.items(), key=lambda (k,v): (v,k), reverse = True):
			print "%s) %s/%s: %s" % (str(i+1), value[0], value[1], key)
			if value[1] == '1':
				os.system( 'flite -t "' + key + '"' )
			i += 1
			if i >= count:
				break
		print '---------------------------------------------------'
		os.system( 'amixer -q set PCM -- 80%' )

# -------------------------------------------------------------------------------------------
os.system( 'amixer -q set PCM -- 50%' )
running = True
pygame.mixer.init()
tlist = topTweets()

# Setup Serial IO to Arduino
ser = serial.Serial('/dev/ttyACM0',115200)

while running:	# Loop Control
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
	



