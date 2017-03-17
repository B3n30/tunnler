//
//  wiredManager.hpp
//  tunnelr
//
//

#ifndef wiredManager_hpp
#define wiredManager_hpp

#include <map>


class wiredManager {
	private:

	public:
		wiredManager();
		int send(const unsigned char*, int length);
		int select();
		std::map<const unsigned char*, int> data;

};

#endif /* wiredManager_hpp */