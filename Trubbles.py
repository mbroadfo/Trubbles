import time, re, pygame, serial
import RPi.GPIO as GPIO
from twython import TwythonStreamer
from termcolor import colored
 
print colored('-----------','yellow')
print colored('hello', 'red'), colored('world', 'blue')
print colored('-----------','yellow')

# Search terms
TERMS = ['@POTUS','@WhiteHouse','@PresSec','@realDonaldTrump','@Reince']
actionList  = (['1','@POTUS','@WhiteHouse','@PressSec','@realDonaldTrump'],
               ['2','@VP','@DeptofDefense','@DHSgov','@CIA','@UN','@StateDept','@USTreasury','@real_sessions','@HHSGov','@USChamber'],
               ['3','@SenateMajLdr','@SpeakerRyan','@GOPLeader','@GOP'],
               ['4','@FoxNews','@foxheadlines','@foxnewsalert','@seanhannity','@oreillyfactor','@foxandfriends'])
actionColor = ['red','yellow','magenta','cyan']

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
				ser.write('h')		#  Blink LED
				reTwit = True		# Initialize
				rtwit = twitt.group(1)
				from1 = twitt.group(2)
				body1 = twitt.group(3)
				body1 = re.sub(r'http\S+','',body1)
				body1 = re.sub('$amp;','&',body1)
				if rtwit is not None:
					from1 = from1 + ' (' + str.strip(rtwit) + ')'
				elif from1 != str.strip(from1):
					print colored('! NEW TWEET !','blue')
					reTwit = False
	                        print 'FROM: ' + from1
				print 'BODY: ' + body1
				tlist.insTweet(from1,body1)	# Capture Tweet
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
								print colored('!!! ACTION ' + actionType + ' !!! - triggered by ' + actor,actionColor[int(actionType)-1])
								if actionType == '1':
									ser.write('abcdef')	# 4 Neopixels + 2 Servos
									if reTwit == False:
										print colored('!!! FROM OUR SUPREME LEADER !!!','blue')
										pygame.mixer.music.load("hail_to_the_chief.mp3")
									else:
										pygame.mixer.music.load("brokenspaceship.mp3")
									pygame.mixer.music.play()
								elif actionType == '2':
									ser.write('abe')	# 2 Neopixels + 1 Servo
									if reTwit == False: 
										print colored('! The Cabinet Speaks !','blue')
										pygame.mixer.music.load("horse.mp3")
									else:
										pygame.mixer.music.load("horse.mp3")
									pygame.mixer.music.play()
								elif actionType == '3':
									ser.write('bcf')	# 2 Neopixels + 1 Servo
									if reTwit == False: 
										print colored('! Legislative News !','blue')
										pygame.mixer.music.load("compute.mp3")
									else:
										pygame.mixer.music.load("compute.mp3")
									pygame.mixer.music.play()
								elif actionType == '4':
									ser.write('d')	# 1 Neopixel
									if reTwit == False: 
										print colored('! An Opinion!','blue')
										pygame.mixer.music.load("clicker.mp3")
									else:
										pygame.mixer.music.load("beep14.mp3")
									pygame.mixer.music.play()
						n += 1
				print '----------------------------------------------'
		b1 = GPIO.input(btn1)
		b2 = GPIO.input(btn2)
		
		if b1 == False:	# Button pressed
			print 'Button 1 (', b1, ') Pressed'
			tlist.releaseTweets(3)
		
		if b2 == False:	# Button pressed
			print 'Button 2 (', b2, ') Pressed'
			tlist.releaseTweets(10)
		
	def on_error(self, status_code, data):
		print 'ERR:' + status_code
# -------------------------------------------------------------------------

class topTweets:
#	Class to store top tweets by actor
	def __init__ (self):
		topTweets.tweetList = {}		# initialize dictionary
	def insTweet(self, fromer, bodyr):
		tweetr = fromer + ':' + bodyr
		if topTweets.tweetList.has_key(tweetr):
			topTweets.tweetList[tweetr] +=1
			print colored("Tweet Again!",'red')
		else:
			topTweets.tweetList[tweetr] = 1
			print "1st Tweet"
	def releaseTweets(self,count):
		i = 0;
		print 'TOP TWEETS!'
		for key, value in sorted(topTweets.tweetList.items(), key=lambda (k,v): (v,k)):
			print "%s: %s" % (key, value)
			i += 1
			if i >= count:
				break
		topTweets.tweetList = {}		# clear dictionary
# -------------------------------------------------------------------------------------------

running = True
pygame.mixer.init()
tlist = topTweets()
btn1 = 22
btn2 = 23

# Setup Serial IO to Arduino
ser = serial.Serial('/dev/ttyACM0',115200)

# Setup GPIO as output
GPIO.setmode(GPIO.BCM)
GPIO.setup(btn1, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(btn2, GPIO.IN, pull_up_down=GPIO.PUD_UP)

while running:	# Loop Control
	# Create Streamer
	try:
		stream = BlinkyStreamer(APP_KEY, APP_SECRET, OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
		stream.statuses.filter(track=TERMS)
	except KeyboardInterrupt:
		running = False
		print colored('\nBye Bye!','cyan')
	except Exception as e:
		print 'ERROR: ' + str(e)
		continue
	




