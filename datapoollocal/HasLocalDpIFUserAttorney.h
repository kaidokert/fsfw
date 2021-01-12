#ifndef FSFW_DATAPOOLLOCAL_HASLOCALDPIFUSERATTORNEY_H_
#define FSFW_DATAPOOLLOCAL_HASLOCALDPIFUSERATTORNEY_H_

class HasLocalDataPoolIF;
class AccessPoolManagerIF;

class HasLocalDpIFUserAttorney {
private:

	static AccessPoolManagerIF* getAccessorHandle(HasLocalDataPoolIF* interface);

	friend class LocalPoolObjectBase;
	friend class LocalPoolDataSetBase;

};


#endif /* FSFW_DATAPOOLLOCAL_HASLOCALDPIFUSERATTORNEY_H_ */
