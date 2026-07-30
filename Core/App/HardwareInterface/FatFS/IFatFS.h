#ifndef FATFS_INTERFACE_H
#define FATFS_INTERFACE_H

class IFatFS {

public:
	virtual ~IFatFS() = default;

	virtual bool mount() = 0;
	virtual bool close() = 0;
	virtual bool open() = 0;
	virtual bool write(const char* msg) = 0;
	virtual bool sync() = 0;
};

#endif
