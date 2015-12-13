#ifndef __DIRLIST_PARAMETER_H__
#define __DIRLIST_PARAMETER_H__

#include <vector>
#include <string>
#include <ostream>

class DirListParameter
{
public:
	DirListParameter() {}
	DirListParameter(const std::string &k, const std::string &v) { setKey(k); setValue(v); }
	DirListParameter(const std::string &k, const std::string &v, const std::string &c) { setKey(k); setValue(v); setComment(c); }
	~DirListParameter() {}

	bool hasComment() const { return !comment.empty(); }
	unsigned int size() const { return items.size(); }

	// base accessor functions; value could be in quotes
	const std::string &getKey() const { return key; }
	const std::string &getValue() const { return value; }
	const std::string &getComment() const { return comment; }
	
	// used to set fundamental and derived values
	void setKey(const std::string &k) { key = k; }
	void setValue(const std::string &v);	// more complicated as this does some string manipulation
	void setComment(const std::string &c) { comment = c; }

	// returns values in native C++ types, unquoted.  index is zero based and is for array values
	int getInt(unsigned int index = 0) const;
	double getDouble(unsigned int index = 0) const;
	const std::string &getString(unsigned int index = 0) const;

	void print() const;	// diagnostic print

private:
	std::string key;
	std::string value;		// stores everything after = and before #
	std::string comment;		// stores everything after #
	std::vector<std::string> items;	// unquoted list of items after =
};

std::ostream& operator << (std::ostream &os, const DirListParameter &x);

#endif
