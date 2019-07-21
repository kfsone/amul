#pragma once
// alias class definitions and function protos

class   Alias:public ALIAS
{
  public:
	static long locate(const char *s);
	static long locate(vocid_t id);
	static inline vocid_t meaning(long num)
	{
		return data->aliasbase[num].means;
	};
};
