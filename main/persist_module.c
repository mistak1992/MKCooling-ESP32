#include "include/persist_module.h"
#include "esp_system.h"

#define MODULE_TAG "PERSIST_MODULE"

static bool is_init = false;
char *head = "/spiffs/";


char* Int2String(int num, char *str)//10进制 
{
	int i = 0;//指示填充str 
	if(num<0)//如果num为负数，将num变正 
	{
		num = -num;
		str[i++] = '-';
	} 
	//转换 
	do
	{
		str[i++] = num%10+48;//取num最低位 字符0~9的ASCII码是48~57；简单来说数字0+48=48，ASCII码对应字符'0' 
		num /= 10;//去掉最低位	
	}while(num);//num不为0继续循环
	
	str[i] = '\0';
	
	//确定开始调整的位置 
	int j = 0;
	if(str[0]=='-')//如果有负号，负号不用调整 
	{
		j = 1;//从第二位开始调整 
		++i;//由于有负号，所以交换的对称轴也要后移1位 
	}
	//对称交换 
	for(;j<i/2;j++)
	{
		//对称交换两端的值 其实就是省下中间变量交换a+b的值：a=a+b;b=a-b;a=a-b; 
		str[j] = str[j] + str[i-1-j];
		str[i-1-j] = str[j] - str[i-1-j];
		str[j] = str[j] - str[i-1-j];
	} 
	
	return str;//返回转换后的值 
}

int String2Int(char *str)//字符串转数字 
{
	char flag = '+';//指示结果是否带符号 
	long res = 0;
	
	if(*str=='-')//字符串带负号 
	{
		++str;//指向下一个字符 
		flag = '-';//将标志设为负号 
	} 
	//逐个字符转换，并累加到结果res 
	while(*str>=48 && *str<57)//如果是数字才进行转换，数字0~9的ASCII码：48~57 
	{
		res = 10*res+  *str++-48;//字符'0'的ASCII码为48,48-48=0刚好转化为数字0 
	} 
 
    if(flag == '-')//处理是负数的情况
	{
		res = -res;
	}
 
	return (int)res;
}

esp_err_t persist_module_init(){
    // ESP_LOGI(MODULE_TAG, "Initializing SPIFFS");
    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(MODULE_TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(MODULE_TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(MODULE_TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        is_init = false;
    }else
    {
        is_init = true;
    }
    return ret;
}

esp_err_t get_persist_basic_info(size_t total, size_t used){
    esp_err_t ret;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(MODULE_TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(MODULE_TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ret;
}

bool check_file_exists(char *file_name, size_t len){
    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_name, &st) == 0) {
        return true;
        // // Delete it if it exists
        // unlink("/spiffs/foo.txt");
    }
    return false;
}

bool rename_file(char *origin_name, char *desc_name){
    // Rename original file
    ESP_LOGI(MODULE_TAG, "Renaming file");
    if (rename(origin_name, desc_name) != 0) {
        ESP_LOGE(MODULE_TAG, "Rename failed");
        return false;
    }
    return true;
}

char* persist_read_file(char *file_name, char *content_value){
    // Open renamed file for reading
    // ESP_LOGI(MODULE_TAG, "Reading file");
    char *tail = file_name;
    char *name = (char *)calloc(strlen(head) + strlen(tail) + 2, sizeof(char));
    strcpy(name, head);
    strcat(name, tail);
    FILE* f = fopen(name, "r");
    free(name);
    if (f == NULL) {
        ESP_LOGE(MODULE_TAG, "Failed to open file for reading");
        content_value = "0";
        return content_value;
    }
    char line[20];
    fgets(line, sizeof(line), f);
    fclose(f);
    content_value = line;
    // ESP_LOGE(MODULE_TAG, "Open file for reading:%s", line);
    return content_value;
}

void persist_write_file(char *file_name, char *content_value){
    // First create a file.
    char *tail = file_name;
    char *name = (char *)calloc(strlen(head) + strlen(tail) + 2, sizeof(char));
    strcpy(name, head);
    strcat(name, tail);
    // ESP_LOGI(MODULE_TAG, "Opening file %s", name);
    check_file_exists(name, sizeof(name));
    FILE* f = fopen(name, "w");
    free(name);
    if (f == NULL) {
        ESP_LOGE(MODULE_TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, content_value);
    fclose(f);
    // ESP_LOGI(MODULE_TAG, "File written");
}

void persist_module_deinit(){
    esp_vfs_spiffs_unregister(NULL);
    // ESP_LOGI(MODULE_TAG, "SPIFFS unmounted");
    is_init = false;
}

void persist_set_data(enum persist_module_type type, uint16_t value){
    if (is_init == false)
    {
        persist_module_init();
    }
    switch (type){
    case PERSIST_MODULE_DUTY:{
        char value_str[8] = {};
        persist_write_file("fan_duty", Int2String(value, value_str));
        break;
    }
    case PERSIST_MODULE_MLX90614_SWITCH:{
        char value_str[8] = {};
        persist_write_file("MLX90614_SWITCH", Int2String(value, value_str));
        break;
    }
    case PERSIST_MODULE_AUTH:{
        char value_str[8] = {};
        persist_write_file("AUTH", Int2String(value, value_str));
        break;
    }
    default:
        break;
    }
}

char* persist_get_data(enum persist_module_type type, char *value){
    if (is_init == false)
    {
        persist_module_init();
    }
    switch (type){
    case PERSIST_MODULE_DUTY:{
        char *value_str = NULL;
        value = persist_read_file("fan_duty", value_str);
        break;
    }
    case PERSIST_MODULE_MLX90614_SWITCH:{
        char *value_str = NULL;
        value = persist_read_file("MLX90614_SWITCH", value_str);
        break;
    }
    case PERSIST_MODULE_AUTH:{
        char *value_str = NULL;
        value = persist_read_file("AUTH", value_str);
        break;
    }
    default:
        break;
    }
    return value;
}