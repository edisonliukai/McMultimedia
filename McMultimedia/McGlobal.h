#ifndef _MC_GLOBAL_H_
#define _MC_GLOBAL_H_

// ����ɾ��ָ�룬ɾ��ǰ���ж�ָ���Ƿ����
#define MC_SAFE_DELETE(p) (p) ? ((delete p), (p = nullptr)) : (true)

// ��������ָ�����������ж�ָ���Ƿ����
#define MC_SAFE_CALL(p, func) (p) ? (p->func) : (false)

// ѭ�����ã���������������ָ���ĺ����������ڵ�����ֻ������ͨ����
#define MC_LOOP_CALL(container, func)	\
	for (const auto &c : container) {	\
		static_assert(!std::is_pointer<std::decay<decltype(c)>::type>::value, "Data type in container must not be a pointer type");	\
		c.func;	\
	}

// ͬ�ϣ������ڵ�����ֻ����ָ��
#define MC_LOOP_CALL_P(container, func)	\
	for (const auto &c : container) {	\
		static_assert(std::is_pointer<std::decay<decltype(c)>::type>::value, "Data type in container must be a pointer type");	\
		c->func;	\
	}

#endif // !_MC_GLOBAL_H_

