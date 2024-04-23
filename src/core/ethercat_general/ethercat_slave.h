/**
 * @brief Contains ethercat slave properties and methods.
 * Member of EthercatSlavesContainer.
 * @details Configuraton example:
 * @code
 * #include "ethercat_slave_names.h"
 * #include "coe_object_names.h"
 * #include "sdo_list.h"
 * #include "pdo_entries_list.h"
 * #include "default_sync_info.h"
 * #include "ethercat_slave.h"
 * 
 * void ConfigureSlave()
 * {
 * 		// Values coe_object_names::kObjectName1...6 should be
 * 		// previously defined in coe_object_names namespace.
 * 		// Using other std::string values as CoE object names is
 * 		// not recommended.
 * 		PDOEntriesList* rxpdo = new PDOEntriesList();
 * 		rxpdo->AddEntry(coe_object_names::kObjectName1, 0x7001, 0x00, 16);
 * 		rxpdo->AddEntry(coe_object_names::kObjectName2, 0x7002, 0x01, 8);
 * 		
 * 		PDOEntriesList* txpdo = new PDOEntriesList();
 * 		txpdo->AddEntry(coe_object_names::kObjectName3, 0x6001, 0x01, 32);
 * 		txpdo->AddEntry(coe_object_names::kObjectName4, 0x6001, 0x02, 32);
 * 
 * 		DefaultSyncInfo* sync = new DefaultSyncInfo();
 * 		sync->AddRxPDO(0x1600, rxpdo);
 * 		sync->AddTxPDO(0x1A00, txpdo);
 * 		sync->Create();
 * 
 * 		SDOList* sdo_parameters = new SDOList();
 * 		sdo_parameters->AddObject(coe_object_names::kObjectName5, 0x7015, 0x02, 16, -50000);
 * 		sdo_parameters->SetTimeout(500);
 * 		SDOList* sdo_telemetry = new SDOList();
 * 		sdo_telemetry->AddObject(coe_object_names::kObjectName6, 0x6020, 0x00, 8, 0b00010111);
 * 		sdo_telemetry->SetTimeout(500);
 * 
 * 		EthercatSlave* slave = new EthercatSlave();
 * 		// Value ethercat_slave_names::kSomeEthercatSlaveName
 * 		// should be previously defined in ethercat_slave_names namespace.
 * 		// Using other std::string values as slave names is not recommended.
 * 		slave->SetSlaveInfo(ethercat_slave_names::kSomeEthercatSlaveName, 0, 0, 0x5555, 0xAAAA);
 * 		slave->RegisterSync(sync);
 * 		slave->RegisterParameterSDO(sdo_parameters);
 * 		slave->RegisterTelemetrySDO(sdo_telemetry);
 * 		// If using distributed clocks
 * 		slave->SetAssignActivate(0x0300) 	
 * }
 * 
 * @endcode
 * @see EthercatSlavesContainer, SyncInfo, DefaultSyncInfo, SDOList, PDOEntriesList.
*/

#ifndef ETHERCAT_SLAVE_H
#define ETHERCAT_SLAVE_H

#include <map>
#include <string>
#include "coe_object.h"
#include "sync_info.h"
#include "sdo_list.h"
#include "ecrt.h"
#include "coe_drive_state_handler.h"

class EthercatSlave
{ 
	std::string name{""};
	uint16_t alias{0};
	uint16_t position{0};
	uint32_t vendor_id{0};
	uint32_t product_code{0};
	
	ec_slave_config_t* slave_config = nullptr;
	SyncInfo* sync_info = nullptr;
	SDOList* sdo_parameters = nullptr;
	SDOList* sdo_telemetry = nullptr;
	uint32_t assign_activate = 0;
public:
	EthercatSlave() = default;
	virtual ~EthercatSlave();
	
	/**
	 * @brief Gets IgH object ec_slave_config_t*.
	 * @warning Must NOT be called by user!
	*/
	ec_slave_config_t* GetConfig();
	/**
	 * @brief Gets slave's name.
	 * @returns slave name.
	*/
	std::string GetName();
	/**
	 * @brief Gets slave's alias.
	 * @returns alias.
	*/
	uint16_t GetAlias();
	/**
	 * @brief Gets slave's position.
	 * @returns position.
	*/
	uint16_t GetPosition();
	/**
	 * @brief Gets slave's vendor id.
	 * @returns vendor id.
	*/
	uint32_t GetVendorID();
	/**
	 * @brief Gets slave's product code.
	 * @returns product code.
	*/
	uint32_t GetProductCode();
	/**
	 * @brief Gets slave's assign activate
	 *  (previously passed with SetAssignActivate(uint32_t))
	 * @see SetAssignActivate(uint32_t)
	 * @returns assign activate
	*/
	uint32_t GetAssignActivate();
	/**
	 * @brief Gets pointer to current SyncInfo instance.
	 * @returns current SyncInfo pointer.
	 * @warning Must NOT be called by user!
	*/
	SyncInfo* GetSync();
	/**
	 * @brief Gets pointer to current parameter SDOList.
	 * @returns current parameter SDOList pointer.
	 * @warning Must NOT be called by user!
	*/
	SDOList* GetParameterSDO();
	/**
	 * @brief Gets pointer to current telemetry SDOList.
	 * @returns current telemetry SDOList pointer.
	 * @warning Must NOT be called by user!
	*/	
	SDOList* GetTelemetrySDO();
	/**
	 * @brief Registers previously configured sync with required PDOs.
	 * @param[in] sync configured sync.
	*/
	void RegisterSync(SyncInfo* sync);
	/**
	 * @brief Registers paramater SDOList.
	 * @details Each of CoE objects in SDOList with defined value will be recieved
	 * by ethercat slave once during MaiboxWritingState mailbox task in real-time context after calling 
	 * EthercatThreadManager::StartThread() method.
	 * @param[in] sdos parameter SDOs container.
	 * @see EthercatThreadManager::StartThread(), MailboxManager, MailboxWritingState
	*/
	void RegisterParameterSDO(SDOList* sdos);
	/**
	 * @brief Registers telemetry SDOList.
	 * @details Each of CoE objects in SDOList will update it's value cyclically 
	 * during MaiboxReadingState mailbox. Task will be performing after completing
	 * initial MailboxWritingState mailbox task and therefore in real-time context after calling 
	 * EthercatThreadManager::StartThread() method and before other task is recieved by mailbox manager.
	 * @param[in] sdos telemetry SDOs container.
	 * @see EthercatThreadManager::StartThread(), MailboxManager, MailboxReadingState, MailboxWritingState, RegisterParameterSDO.
	*/
	void RegisterTelemetrySDO(SDOList* sdos);
	/**
	 * @brief Set general slave information.
	 * @param[in] name slave's name.
	 * @param[in] alias slave's alias.
	 * @param[in] position slave's position.
	 * @param[in] vendor_id slave's vendor id.
	 * @param[in] product_code slave's product code.
	*/
	void SetSlaveInfo(std::string name, uint16_t alias, uint16_t position, uint32_t vendor_id, uint32_t product_code);
	/**
	 * @brief Set assign activate value.
	 * @details Necessary parameter for operating slave
	 * with distributed clocks.
	 * @param[in] assign_activate assign activate value.
	*/
	void SetAssignActivate(uint32_t assign_activate);

	CoEObject* GetTxPDOEntry(std::string);
	CoEObject* GetRxPDOEntry(std::string);
	CoEObject* GetParameterSDOEntry(std::string);
	CoEObject* GetTelemetrySDOEntry(std::string);

	void Configure(ec_master_t*);
	void CreatePDO();
	void CreateSDO();
};

#endif