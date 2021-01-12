#ifndef FSFW_DATAPOOLLOCAL_HASLOCALDPIFUSERATTORNEY_H_
#define FSFW_DATAPOOLLOCAL_HASLOCALDPIFUSERATTORNEY_H_

class AccessPoolManagerIF;
class HasLocalDataPoolIF;

class HasLocalDpIFUserAttorney {
private:

	static AccessPoolManagerIF* getAccessorHandle(HasLocalDataPoolIF* interface) {
		return interface->getAccessorHandle();
	}

	friend class LocalPoolObjectBase;
	friend class LocalPoolDataSetBase;

};


#endif /* FSFW_DATAPOOLLOCAL_HASLOCALDPIFUSERATTORNEY_H_ */
