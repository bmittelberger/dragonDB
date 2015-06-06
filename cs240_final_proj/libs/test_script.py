import plyvel
db = plyvel.DB('/tmp/hello_world/', create_if_missing=True)
key = 'hello world'
db.put(key, b'<html><body><h1>Hello World</h1></body></html>')
print db.get(key)
