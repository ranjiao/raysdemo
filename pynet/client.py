from mud import MUDClient

client =  MUDClient.MUDClient()
client.connect("localhost", 3742)
client.login('user1', 'passwd1')
