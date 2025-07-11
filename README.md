# bc150 testing

/testing_bc150 contains some scripts that can test udp communications with the bc150 modems

`udp_test_runner.py` 

sends a handful of commands and is the primary test script

some messages have multiple responses so it accepts up to 5

Need to use 10 (GUI) as the sender/source so that the modem returns the responses to the sender's ip address
The first command in `test_cases.py` sets the GUI address to your own. modify it as necessary

`test_cases.py` contains the chronological list of test messages. expand on this to cover as many cases as possible

test cases should also probably contain a field for the anticipated response msg identifier. 
this will help to evaluate the success of each message

TODO:
- [ ] Add expected responses to the commands
- [ ] Build out the message testing coverage
- [ ] Include cases that are expected to fail
- [ ] Use assorted message contents that could potentially break the code (negative nums, 0, overflows, decimals, words, etc.)
- [ ] Generate a fails/success report at the end of the test


`udp_test_listener.py` 

is a script that ssh's into the modem using the credentials in the .env file
it then uses tcpdump to record messages sent to the localhost 127.0.0.1 address
these messages are intended for the webserver interface

Could be useful to get responses that are not currently returned to the source/sender
