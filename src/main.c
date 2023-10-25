#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "wasm3.h"
#include "m3_core.h"
#include "m3_env.h"
#include "hello.wasm.h"

#define WASM_STACK_SIZE 1024
#define WASM_MEMORY_LIMIT 4096

m3ApiRawFunction(GPIO_init)
{
    m3ApiGetArg(uint, gpio);

    gpio_init(gpio);
    printf("Initialized GPIO %d\n", gpio);

    m3ApiSuccess();
}

m3ApiRawFunction(GPIO_set_dir)
{
    m3ApiGetArg(uint, gpio);
    m3ApiGetArg(bool, out);

    gpio_set_dir(gpio, out);
    printf("Set GPIO %d to %s\n", gpio, out ? "out" : "in");

    m3ApiSuccess();
}

m3ApiRawFunction(GPIO_put)
{
    m3ApiGetArg(uint, gpio);
    m3ApiGetArg(bool, value);

    gpio_put(gpio, value);
    printf("Set GPIO %d to %d\n", gpio, value);

    m3ApiSuccess();
}

m3ApiRawFunction(Time_get)
{
    m3ApiReturnType(uint64_t);

    uint64_t time = to_us_since_boot(get_absolute_time());

    m3ApiReturn(time);
}

int main()
{
    stdio_init_all();

    // Wait for a connection to the serial port to be established
    while (!tud_cdc_connected())
    {
    }

    // Say hi to the host
    puts("Hello, world!");

    // Initialize a new WASM3 environment and runtime
    IM3Environment env = m3_NewEnvironment();
    IM3Runtime runtime = m3_NewRuntime(env, WASM_STACK_SIZE, NULL);
    runtime->memoryLimit = WASM_MEMORY_LIMIT; // Set the memory limit to 4KB

    // Parse the previously generated WASM module
    IM3Module module = {0};
    M3Result result = m3_ParseModule(env, &module, hello_wasm, hello_wasm_len);
    if (result)
    {
        printf("Error parsing WASM module: %s\n", result);
        goto end;
    }
    printf("Parsed WASM module\n");

    // Load the WASM module into the runtime
    result = m3_LoadModule(runtime, module);
    if (result)
    {
        printf("Error loading WASM module: %s\n", result);
        goto end;
    }
    printf("Loaded WASM module\n");

    // Link basic GPIO interacting and time functions to the module
    m3_LinkRawFunction(module, "GPIO", "gpio_init", "v(i)", &GPIO_init);
    m3_LinkRawFunction(module, "GPIO", "gpio_set_dir", "v(ii)", &GPIO_set_dir);
    m3_LinkRawFunction(module, "GPIO", "gpio_put", "v(ii)", &GPIO_put);
    m3_LinkRawFunction(module, "Time", "get", "I()", &Time_get);
    printf("Linked raw functions\n");

    // Find the main function of the module
    IM3Function function = {0};
    result = m3_FindFunction(&function, runtime, "main");
    if (result)
    {
        printf("Error trying to find main function: %s\n", result);
        goto end;
    }
    printf("Found main function\n");

    // Call the main function
    m3_CallV(function);

    // Free the resources (this should never happen since our example WASM program runs an infinite loop but who knows)
    m3_FreeModule(module);
    m3_FreeRuntime(runtime);
    m3_FreeEnvironment(env);
    printf("Done!\n");

end:
    while (true)
    {
    }

    return 0;
}
