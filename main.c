/*-[ I2C Bus Scanning ]-*/
HAL_UART_Transmit(&hlpuart1, StartMSG, sizeof(StartMSG), 10000);
for(i=1; i<128; i++)
{
    ret = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i<<1), 3, 5);
    if (ret != HAL_OK) /* No ACK Received At That Address */
    {
        HAL_UART_Transmit(&hlpuart1, Space, sizeof(Space), 10000);
    }
    else if(ret == HAL_OK)
    {
        sprintf(Buffer, "0x%X", i);
        HAL_UART_Transmit(&hlpuart1, Buffer, sizeof(Buffer), 10000);
    }
}
HAL_UART_Transmit(&hlpuart1, EndMSG, sizeof(EndMSG), 10000);
/*--[ Scanning Done ]--*/



