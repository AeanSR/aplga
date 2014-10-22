#include "aplga.h"

char* ocl_src_char;

int ocl_t::init()
{
    std::cout << "Query available compute devices ...\n";

    cl_int err;
    cl_uint num;
    err = clGetPlatformIDs(0, 0, &num);
    if (err != CL_SUCCESS) {
        std::cerr << "Unable to get platforms\n";
        return 0;
    }
    std::cout << "num of platforms " << num << "\n";
    std::vector<cl_platform_id> platforms(num);
    err = clGetPlatformIDs(num, &platforms[0], &num);
    if (err != CL_SUCCESS) {
        std::cerr << "Unable to get platform ID\n";
        return 0;
    }

    cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0 };
    context = clCreateContextFromType(prop, CL_DEVICE_TYPE_ALL, NULL, NULL, NULL);
    if (context == 0) {
        std::cerr << "Can't create OpenCL context\n";
        return 0;
    }

    size_t dev_c, info_c;
    clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &dev_c);
    std::vector<cl_device_id> devices(dev_c / sizeof(cl_device_id));
    clGetContextInfo(context, CL_CONTEXT_DEVICES, dev_c, &devices[0], 0);

    for (auto i = devices.begin(); i != devices.end(); i++){
        clGetDeviceInfo(*i, CL_DEVICE_NAME, 0, NULL, &info_c);
        std::string devname;
        devname.resize(info_c);
        clGetDeviceInfo(*i, CL_DEVICE_NAME, info_c, &devname[0], 0);
        std::cout << "\tDevice " << i - devices.begin() + 1 << ": " << devname.c_str() << "\n";
    }
    std::cout << "OK!\n";

    queue = clCreateCommandQueue(context, devices[0], 0, 0);
    if (queue == 0) {
        std::cerr << "Can't create command queue\n";
        return 0;
    }

    cl_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float)* iterations, NULL, NULL);
    if (cl_res == 0) {
        std::cerr << "Can't create OpenCL buffer\n";
        return 0;
    }

    FILE* f = fopen("krnl.c", "rb");
    fseek(f, 0, SEEK_END);
    size_t tell = ftell(f);
    rewind(f);
    ocl_src_char = (char*)calloc(tell + 1, 1);
    fread(ocl_src_char, tell, 1, f);

    return 0;
}

float ocl_t::run(std::string& apl_cstr){
    cl_int err;

    std::cout << "JIT ...\r";

    std::string source(ocl_src_char);
    source.append("void scan_apl( rtinfo_t* rti ) {");
    source.append(apl_cstr);
    source.append("}");
    const char* cptr = source.c_str();

    cl_program program = clCreateProgramWithSource(context, 1, &cptr, 0, 0);
    if (program == 0) {
        return -1.0;
    }

    if (clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) {
        std::cerr << "Can't build program\n";
        return -1.0;
    }
    if (program == 0) {
        std::cerr << "Can't load or build program\n";
        return -1.0;
    }

    cl_kernel sim_iterate = clCreateKernel(program, "sim_iterate", 0);
    if (sim_iterate == 0) {
        std::cerr << "Can't load kernel\n";
        clReleaseProgram(program);
        return -1.0;
    }

    clSetKernelArg(sim_iterate, 0, sizeof(cl_mem), &cl_res);

    size_t work_size = iterations;
    err = clEnqueueNDRangeKernel(queue, sim_iterate, 1, 0, &work_size, 0, 0, 0, 0);

    float* res = new float[iterations];
    float ret;

    std::cout << "Sim ...\r";
    if (err == CL_SUCCESS) {
        err = clEnqueueReadBuffer(queue, cl_res, CL_TRUE, 0, sizeof(float) * iterations, &res[0], 0, 0, 0);
        if (err != CL_SUCCESS){
            printf("Can't read back data %d\n", err);
            ret = -1.0;
        }else{
            ret = prefix_mean(res, iterations);
            std::cout << "       \r";
        }
    }
    else{
        printf("Can't run kernel %d\n", err);
        ret = -1.0;
    }
    delete[] res;
    clReleaseKernel(sim_iterate);
    clReleaseProgram(program);
    return ret;
}

int ocl_t::free(){
	clReleaseMemObject(cl_res);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
	return 0;
}

ocl_t& ocl(){
    static ocl_t s;
    return s;
}

/*
int main(){
    std::string apla = "SPELL(bloodthirst); SPELL(execute); SPELL(ragingblow); SPELL(wildstrike);";
    std::string aplb = "SPELL(bloodthirst); SPELL(wildstrike); SPELL(ragingblow); SPELL(execute);";
    while (1){
        std::cout << ocl().run(apla) << "\n";
        std::cout << ocl().run(aplb) << "\n";
    }
}
*/
