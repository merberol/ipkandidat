#pragma once


namespace HapticInterface {
	template <class T>
	class DataWrapper {
	protected:
		T Value;
	public:
		DataWrapper() 
		:Value{}
		{}

		DataWrapper(T val)
			:Value{val}
		{}

		T get() {
			return (*this).Value;
		}

		void set(T val) {
			(*this).Value = val;
		}
	};


	class HIInt : public  DataWrapper<int> {
		HIInt(int val) 
		:DataWrapper<int>{val}
		{

		}
	};
}