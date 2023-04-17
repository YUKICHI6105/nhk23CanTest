#include <can.h>

#include <CRSLibtmp/Can/Stm32/RM0008/filter_manager.hpp>
#include <CRSLibtmp/Can/Stm32/RM0008/can_bus.hpp>
#include <CRSLibtmp/Can/Stm32/RM0008/utility.hpp>

using namespace CRSLib::IntegerTypes;
using namespace CRSLib::Can::Stm32::RM0008;
using CRSLib::Can::DataField;

const char * error_msg{nullptr};

void init_can_msp_filter(CAN_HandleTypeDef *const hcan)
{
	// ピンなど初期化
	HAL_CAN_DeInit(hcan);
	HAL_CAN_MspInit(hcan);

	FilterConfig fc[1];
	fc[0] = FilterConfig{};

	FilterManager::initialize(filter_bank_size, fc);
	if(!FilterManager::set_filter(0, FilterManager::make_mask32(0x300, 0x7FF)))
	{
		error_msg = "Filter cannot set.";
		Error_Handler();
	}
	FilterManager::activate(0);
}

DataField data{.buffer={}, .dlc=3};

extern "C" void main_cpp(CAN_HandleTypeDef *const hcan) noexcept
{
	init_can_msp_filter(hcan);

	CanBus can_bus{can1};

	while(true)
	{
		if(const auto message = can_bus.receive(Fifo::Fifo0); message)
		{
			data = message->data;
		}

		DataField tx_data;
		tx_data.buffer[0] = (byte)'H';
		tx_data.buffer[1] = (byte)'E';
		tx_data.buffer[2] = (byte)'L';
		tx_data.buffer[3] = (byte)'L';
		tx_data.buffer[4] = (byte)'O';
		tx_data.buffer[5] = data.buffer[0];
		tx_data.buffer[6] = data.buffer[1];
		tx_data.buffer[7] = data.buffer[2];

		(void)can_bus.post(0x301, tx_data);
	}
}
