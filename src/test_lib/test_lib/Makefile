
SHELL=bash
#--whole-archive 和 --no-whole-archive 可以让gcc把没有被调用函
#数也链接进可执行文件
#CFLAGS += -DDEBUG_LOG -Wl,--whole-archive,--no-whole-archive CFLAGS += -DDEBUG_LOG 
ProjConf = proj_conf
CurDirPrefix = $(shell pwd)
CurDir = $(CurDirPrefix)/bin
CurLib = $(CurDirPrefix)/lib
CurPro = hh
ExistName = $(shell grep $(proj) $(CurDirPrefix)/$(ProjConf))
ExistData = 0
CurTime = $(shell date +%Y_%m_%d_%H_%M_%S)
proj_array=
brep=
rep=
comma = ,
upper_proj = ""
Model = $(model) #LDFLAGS += -L$(CurLib)/comm_lib/ -licudata -licuuc -ltoml

ServerArray = $(Server360)
#ServerArray  = $(GetUrl)
#ServerArray = $(GetUrl) $(Server360) $(SmartHome) 
#ServerArray = $(SmartHome)
#ServerArray = $(UpushServer)
#ServerArray = $(PushServer) $(NatServer) $(UdtServer) $(SmartHome)
CommArray = $(CurLib)

export CurDir CurLib CFLAGS CurPro
		
ifeq ($(proj),)
	ServerArray = $(shell cat $(CurDirPrefix)/$(ProjConf) | tr '\n' ' ')
else
	upper_proj=$(shell echo $(proj) | tr '[a-z]' '[A-Z]')
	rep = $(shell grep -w $(proj) $(CurDirPrefix)/$(ProjConf) | tr '\n' ' ')
endif

comm = $(findstring $(comma),$(proj))
ifneq ($(comm),)
	proj_array=$(shell ./split.sh $(proj) | tr '\n' ' ')
	brep = $(rep)
else
	proj_array=$(rep)
endif

ifneq ($(proj_array),)
	ServerArray = $(proj_array)	
endif


ifeq ($(rep),)
	ExistData = 0
else
	ExistData = 1
endif

define set_proj_info 
	echo ---$(1); \
	rm -rf ./bin/$(1); \
	mkdir -p ./bin/$(1); \
	mkdir -p ./log/$(1); \
	pushd $(1);make proj=$(1);popd; \
	mkdir -p ./log/$(1); \
	mkdir -p ./packet/log/$(1); \
	cp -rf ./bin/$(1)/ ./packet/bin/; \
	cp -rf ./config/$(1)/ ./packet/config; \
	echo ./run.sh -t udt_mobile -p $(1) > ./mobile_$(1).sh; \
	chmod 777 ./mobile_$(1).sh; \
	echo ./run.sh -t udt_gateway -p $(1) > ./gateway_$(1).sh; \
	chmod 777 ./gateway_$(1).sh; \
	mv ./mobile_$(1).sh ./packet; \
	mv ./gateway_$(1).sh ./packet; \
	pushd ./packet;find -name .svn | xargs rm -rf;popd
endef

define copy_config
	@for i in $(1); do \
		cp config/$$i/* packet/config/$$i/; \
	done
endef

define enter_dir
	@echo $(1)
	@for i in $(1);do \
		$(call set_proj_info,$$i); \
	done
endef

define enter_dir_lib
	echo $(1)
	@for i in $(1);do \
		$(call set_proj_info,$$i); \
	done
endef

define enter_dir_lib_old
	@for i in $(1);do \
		echo $$i; \
		make -C $$i; \
		mkdir -p ./packet/log/$$i; \
		cp -rf ./bin/$$i/ ./packet/bin/; \
		cp -rf ./redis/redis_qt ./packet/bin; \
		cp -rf ./config/$$i/ ./packet/config; \
		cp -rf ./config/comm/*.txt ./packet/config/comm; \
		echo ./run.sh -t udt_mobile -p $$i > ./mobile_$$i.sh; \
		chmod 777 ./mobile_$$i.sh; \
		echo ./run.sh -t udt_gateway -p $$i > ./gateway_$$i.sh; \
		chmod 777 ./gateway_$$i.sh; \
		cp ./mobile_$$i.sh ./packet; \
		cp ./gateway_$$i.sh ./packet; \
		cp -rf ./packet_lib/lib/comm_lib/*.so* ./packet/lib/comm_lib; \
		pushd ./packet;find -name .svn | xargs rm -rf;popd; \
	done
endef

define clean_dir
	@for i in $(1);do \
		pushd $$i;make proj=$$i clean;popd; \
		rm -rf packet_$$i.tar.xz; \
		rm -rf packet_lib_$$i.tar.xz; \
	done
endef

define create_new_packet
	rm -rf packet
	mkdir -p packet
	cp run.sh packet
	mkdir -p ./packet/lib/comm_lib
	mkdir -p ./packet/config/comm
	mkdir -p ./packet/bin/
	mkdir -p ./packet/log/
	cp -rf ./config/comm/* ./packet/config/comm
endef

define tar_packet
	rm -rf packet.tar.xz
	tar -Jcf packet.tar.xz packet
endef

define tar_packet_lib
	cp ./lib/comm_lib/*.so* ./packet/lib/comm_lib/
	rm -rf packet_lib.tar.xz
	tar -cJf packet_lib.tar.xz packet
endef

define create_new_project_old
	@proj=$(1); \
	mkdir -p $(proj); \
	cp ./project_model/proj_root_makefile $(proj)/Makefile; \
	pushd $(proj);mkdir doc; mkdir $(proj);cp ../project_model/proj_src_makefile $(proj)/Makefile; popd; \
	echo $(proj) >> $(CurDirPrefix)/$(ProjConf)
endef

define create_new_project_ok
	data=""
	proj=$(1)
	echo $(ExistName)
	for i in $(ExistName);do \
		if [ "$$i" -eq "$(proj)" ];then \
			data=$(proj) \
		fi \
	done
	mkdir -p $(proj)
	echo $(data)
	if [ $(data) -eq "" ];then
		cp ./project_model/proj_root_makefile $(proj)/Makefile
		pushd $(proj);mkdir doc; mkdir $(proj);cp ../project_model/proj_src_makefile $(proj)/Makefile; popd
		echo $(proj) >> $(CurDirPrefix)/$(ProjConf)
	fi
endef


define create_new_project
	proj=$(1)
	@if [ $(ExistData) -eq 0 ];then \
		mkdir -p $(proj); \
		mkdir -p ./config/$(proj); \
		mkdir -p ./bin/$(proj); \
		mkdir -p ./log/$(proj); \
		cp ./project_model/proj_lib_gateway_config.toml ./config/$(proj)/gateway.toml; \
		cp ./project_model/proj_lib_mobile_config.toml ./config/$(proj)/mobile.toml; \
		cp ./project_model/proj_root_makefile $(proj)/Makefile; \
		pushd $(proj);mkdir doc; mkdir $(proj);cp ../project_model/proj_src_makefile $(proj)/Makefile; popd; \
		echo "#ifndef TC_$(upper_proj)_$(CurTime)_H" > $(proj)/$(proj)/$(proj).h; \
		echo "#define TC_$(upper_proj)_$(CurTime)_H" >> $(proj)/$(proj)/$(proj).h; \
		if [ ""$(model) != "" ];then \
			cat ./project_model/$(model)/proj_test_model.h >> $(proj)/$(proj)/$(proj).h; \
		fi; \
		echo "#endif" >> $(proj)/$(proj)/$(proj).h; \
		echo "#include \"$(proj).h\"" > $(proj)/$(proj)/$(proj).c; \
		if [ ""$(model) != "" ];then \
			cat ./project_model/$(model)/proj_test_model.c >> $(proj)/$(proj)/$(proj).c; \
		fi; \
		echo $(proj) >> $(CurDirPrefix)/$(ProjConf); \
		echo $(proj) >> ./last_proj.txt; \
	fi
endef

define create_script
	for i in $(1); do \
		echo "make proj=$$i pro_lib" > lib_$$i.sh; \
		chmod 777 lib_$$i.sh; \
		echo "make proj=$$i" > norm_$$i.sh; \
		chmod 777 norm_$$i.sh; \
		echo "make proj=$$i test" > test_$$i.sh; \
		chmod 777 test_$$i.sh;\
		echo "make proj=$$i clean" > clean_$$i.sh; \
		chmod 777 clean_$$i.sh; \
	done
endef

define create_new_lib
	proj=$(1)
	@if [ "$(proj)" == "" ];then \
		echo "sdfsdfsf"; \
		make no_$(proj);\
	fi
	@if [ -d ./lib/$(proj) ];then  \
		echo ./lib/$(proj) has existed; \
		make no_$(proj) exited; \
	else \
		mkdir -p ./lib/$(proj)/$(proj); \
	fi
	echo $(proj) >> ./lib/lib_conf
	cp ./project_model/proj_root_makefile ./lib/$(proj)/Makefile
	echo ./lib/$(proj) >> last_proj.txt
	cp ./project_model/proj_lib_src_makefile ./lib/$(proj)/$(proj)/Makefile
	@if [ ""$(model) != "" ];then \
		echo "#ifndef TC_$(upper_proj)_$(CurTime)_H" > ./lib/$(proj)/$(proj)/$(proj).h; \
		echo "#define TC_$(upper_proj)_$(CurTime)_H" >> ./lib/$(proj)/$(proj)/$(proj).h; \
		cat ./project_model/$(model)/proj_test_model.h >> ./lib/$(proj)/$(proj)/$(proj).h; \
		echo "#endif" >> ./lib/$(proj)/$(proj)/$(proj).h; \
		echo "#include \"$(proj).h\"" > ./lib/$(proj)/$(proj)/$(proj).c; \
		cat ./project_model/$(model)/proj_test_model.c >> ./lib/$(proj)/$(proj)/$(proj).c; \
	else \
		echo "#ifndef TC_$(upper_proj)_$(CurTime)_H" > ./lib/$(proj)/$(proj)/$(proj).h; \
		echo "#define TC_$(upper_proj)_$(CurTime)_H" >> ./lib/$(proj)/$(proj)/$(proj).h; \
		echo "#endif" >> ./lib/$(proj)/$(proj)/$(proj).h; \
		echo "#include \"$(proj).h\"" > ./lib/$(proj)/$(proj)/$(proj).c; \
	fi
	

endef	

all:
	echo $(ServerArray)
	$(call create_new_packet,)
	$(call enter_dir, $(ServerArray))
	$(call tar_packet,)

help:
	@echo "make pro_lib 		--- 生成带有动态库的.xz压缩包"
	@echo "make test 		--- 编译所有项目，并把所有项目的配置可科执行文件都拷贝到调试目录(packet)中"
	@echo "make test [proj=xxx]   	--- 仅仅编译xxx项目，并把该项目的配置和可执行文件均拷贝到调试目录(packet)中"
	@echo "make script 		--- 生成所有项目的执行脚本"
	@echo "make script [proj=xxx]	--- 生成xxx项目的执行shell脚本" 
	@echo "make ldlib 		--- 编译lib目录下的所有库"
	@echo "make new proj=xxx 	--- 生成新项目xxx目录"
	@echo "make del proj=xxx 	--- 删除xxx项目的目录"
	@echo "make new_lib proj=xxx 	--- 生成新的库目录"
	@echo "make del_lib proj=xxx 	--- 删除lib目录下的xxx目录"
	@echo "make clean 		--- 清楚所有项目的编译结果"
	@echo "make clean proj=xxx 	--- 清楚xxx项目的编译结果"
	@echo "make lib_clean 		--- 清除lib目录下的编译结果"

pro_lib:
	echo ---$(ServerArray)
	$(call create_new_packet,)
	$(call enter_dir, $(ServerArray))
	$(call tar_packet_lib,)
test:
	$(call enter_dir, $(ServerArray))
	cp run.sh ./packet/
	cp ./lib/comm_lib/*.so* ./packet/lib/comm_lib/

cconfig:
	$(call copy_config, $(ServerArray))

#	cp ./lib/comm_lib/liblightcp_client.so ./packet/lib/comm_lib/
script:
	$(call create_script, $(ServerArray))

push_server:
	make -C $(PushServer)
	
nat_server:
	make -C $(NatServer)

udt_server:
	make -C $(UdtServer)

sync:
	make -C $(SyncServer)

360:
	make -C $(Server360)
smart:
	make -C $(SmartHome)
upush:
	make -C $(UpushServer)

ldlib:
	@for i in $(CurLib); do \
		echo $$i; \
		make -C $$i proj=$(proj); \
	done

new:
	@echo "upper_proj = $(upper_proj)"
	$(call create_new_project,$(proj))
del:
	@if [ ""$(proj) != "" ]; then \
		tar -cJf $(proj).$(CurTime).tar.xz $(proj); \
		rm -rf $(proj); \
		rm -rf ./config/$(proj); \
		rm -rf ./log/$(proj); \
		rm -rf ./bin/$(proj); \
		echo "proj = $(proj)"; \
		sed -i '/\<$(proj)\>/d' $(CurDirPrefix)/$(ProjConf); \
		sed -i '/\<$(proj)\>/d' ./last_proj.txt; \
	fi
	
new_lib: 
	@echo "dfsdf"
	$(call create_new_lib,$(proj)) 
del_lib:
	@if [ ""$(proj) != "" ];then \
		tar -cJf ./lib/$(proj).$(CurTime).tar.xz ./lib/$(proj); \
		rm -rf ./lib/$(proj); \
		sed -i '/\<$(proj)\>/d' ./lib/lib_conf; \
		sed -i '/\<$(proj)\>/d' ./last_proj.txt; \
	fi

clean:
	echo $(ServerArray)
	echo "proj = $(proj)"
	echo "rep = $(rep)"
	echo "proj_array = $(proj_array)"
	echo "brep = $(brep)"
	echo "comm = $(comm)"
	$(call clean_dir, $(ServerArray))

push_clean:
	make -C $(PushServer) clean

nat_clean:
	make -C $(NatServer) clean

udt_clean:
	make -C $(UdtServer) clean
sync_clean:
	make -C $(SyncServer) clean
360_clean:
	make -C $(Server360) clean

lib_clean:
	make -C lib clean proj=$(proj)

