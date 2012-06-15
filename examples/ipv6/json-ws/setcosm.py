#!/usr/bin/python

# python set time code
import httplib,sys

# edit the key and feed parameters to match your COSM account and feed
key = "<your-key>"
feed = "<your-feed>"
cosmaddr = "[2001:470:1f10:333::2]"

print "JSON-WS COSM configuration utility\n   Currently set to COSM feed: %s Key: '%s'" % (feed, key)
if len(sys.argv) > 2:
    host = sys.argv[1]
    stream = sys.argv[2]
else:
    print "Usage: ", sys.argv[0], "<host> <feed-id>"
    sys.exit()

print "Setting cosm config at:", host, " feed:", feed, " stream:",stream

conn = httplib.HTTPConnection(host)
# NAT64 address =
#conn.request("POST","", '{"host":"[2001:778:0:ffff:64:0:d834:e97a]","port":80,"path":"/v2/feeds/55180/datastreams/1","interval":120}')

requestData = '{"host":"%s","port":80,"path":"/v2/feeds/%s/datastreams/%s","appdata":"%s","interval":120,"proto":"cosm"}' % (cosmaddr, feed, stream, key)
print "Posting to node: ", requestData
conn.request("POST","", requestData)

res = conn.getresponse()
print res.status, res.reason
