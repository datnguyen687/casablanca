I. Dependencies:
a. CMake:
_ Download latest version of CMake from https://cmake.org/download/ (suggested >=3.6.2)
_ To build from source, follow https://gitlab.kitware.com/cmake/cmake
b. CLion:
_ Download from https://www.jetbrains.com/clion/
c. MongoDB C++
_ Follow this https://mongodb.github.io/mongo-cxx-driver/mongocxx-v3/installation/ for more detail.
_ If building from source code, uncompress the downloaded file (or git clone from its repos), create build folder if not found, enter build folder then:
	+ cmake -DCMAKE_BUILD_TYPE=Release
	+ make
	+ sudo make install
_ To test if install successfully, run mongo --version, it should be like:
	MongoDB shell version v3.4.4
	git version: 888390515874a9debd1b6c5d36559ca86b44babd
	OpenSSL version: OpenSSL 1.0.2k  26 Jan 2017
	allocator: system
	modules: none
	build environment:
    		distarch: x86_64
    		target_arch: x86_64
_ Also run, mongod --version, it should be like:
	OpenSSL version: OpenSSL 1.0.2k  26 Jan 2017
	allocator: system
	modules: none
	build environment:
    		distarch: x86_64
   		target_arch: x86_64
d. Casablanca
_ Follow https://casablanca.codeplex.com/wikipage?title=Setup%20and%20Build%20on%20OSX&referringTitle=Documentation for Mac
e. JsonCpp
_ Use brew install jsoncpp
_ Or follow https://github.com/open-source-parsers/jsoncpp for more detail
f. Wt C++
_ Run git clone git://github.com/kdeforche/wt.git
_ Create a build folder if not found, enter the build folder
_ run:
	+ cmake ..
	+ make
	+ sudo make install
_ If you find any error with Sqlite, you can disable Sqlite using ccmake ..
g. boost
_ Follow this link: http://www.boost.org/doc/libs/1_61_0/more/getting_started/unix-variants.html for fresh installation.

II. Database:
_ This demo will use database at default address (localhost:27017), database name will be casablanca with its collections correspondently: trader, stock, transaction, portfolio
_ To see all database and collections.
	+ sudo mongod
	+ mongo
	+ show dbs
	+ use casablanca
	+ show collections
	+ db.[collection_name].find()

III. Source Code:
a. Server:
_ cd server/build
_ If you want to build again run cmake .. -> make
_ To run the server, ./server
_ Server by default will run at http://localhost:8080, it will support POST method at: /Buy, /Login, /PortfolioList, /Quote, /RegisterTrader, /Sell, /Transactions and GET method at: /ping
_ Server will auto connect to mongodb and create collections if not found (suggested mongod to run before starting server)
_ To test server:
	+ curl http://localhost:8080/ping
	+ curl http://localhost:8080/Quote -X POST -d '{"username":"Dat", "password":"Test", "stockcode":"ABC"}'
	+ curl http://localhost:8080/Sell -X POST -d '{"username":"Dat", "password":"Test", "stockcode":"ABC", "quantity":1, "price":200}'
	+ curl http://localhost:8080/PortfolioList -X POST -d '{"username":"Dat", "password":"Test", "stockcode":"ABC"}'
	+ curl http://localhost:8080/Buy -X POST -d '{"username":"Dat", "password":"Test", "stockcode":"ABC", "quantity":10, "price":20}'
	+ curl http://localhost:8080/Transactions -X POST -d '{"username":"Dat_1", "password":"Test"}'
	+ curl http://localhost:8080/RegisterTrader -X POST -d '{"username":"Dat_1", "password":"Test"}'
	+ curl http://localhost:8080/Login -X POST -d '{"username":"Dat_1", "password":"Test"}'
b. Application:
_ cd application/build
_ If you want to build again run cmake .. -> make
_ To run the application, ./application  --docroot . --http-address 0.0.0.0 --http-port 8088
_ Open web browser with url: http://0.0.0.0:8088/
_ This web page is very basic allow you to Sign Up new trader, buy, sell, see logs, it will automatically login when you want to buy, sell, see logs, …
c. unit_test
_ cd unit_test/build
_ If you want to build again run cmake .. -> make
_ To run the application, ./unit_test (suggest to run mongod/server beforehand)
_ It will automatically create the needed database for testing

IV. Assumption:
a. Price will be price per unit, not total price.
b. totalcost from porfolio collections will be the sum from beginning, not updated.
c. All api will be in POST method

V. Suggestion:
a. Please add some explanation about totalcost, balancecast, …
b. Used to be in Back End stuff so not feeling good with Front End design.
