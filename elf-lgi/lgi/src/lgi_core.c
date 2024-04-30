/*
**
**                      -+- lgi -+-
**
**                 Mundane Graphics API.
**
**  *--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
**  Copyright(C) Dayan Rodriguez, 2022, All Rights Reserved
**  *--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
**
**        NOT PRODUCTION READY/NOT PRODUCTION QUALITY
**
**
**               HONI SOIT QUI MAL Y PENSE
**
**
**                 github.com/MicroRJ/lgi
**
*/

/* todo: this is provisional */
lgi_API void lgi_setClippingZone(int x0, int y0, int x1, int y1) {
	lgi_ASSERT(x0 <= x1);
	lgi_ASSERT(y0 <= y1);

	x0 = iclamp(x0,0,LGI.Window.size_x);
	y0 = iclamp(y0,0,LGI.Window.size_y);
	x1 = iclamp(x1,0,LGI.Window.size_x);
	y1 = iclamp(y1,0,LGI.Window.size_y);

	D3D11_RECT rect_d3d[1];
	rect_d3d[0].left  = x0;
	rect_d3d[0].top   = LGI.Window.size_y - y1;
	rect_d3d[0].right = x1;
	rect_d3d[0].bottom= LGI.Window.size_y - y0;
	ID3D11DeviceContext_RSSetScissorRects(LGI.d3d11.ctx,1,rect_d3d);
}

lgi_API void lgi_returnBufferContents(lgi_Buffer *xx) {
	lgi_ASSERT(xx->mapped.memory != NULL);

	if (xx->mapped.memory != NULL) {
		ID3D11DeviceContext_Unmap(LGI.d3d11.ctx,xx->d3d11.resource,0);
		xx->mapped.memory = NULL;
	}
}

lgi_API void *lgi_borrowBufferContents(lgi_Buffer *xx, int *lpStride) {
	lgi_ASSERT(xx->mapped.memory == NULL);

	if (xx->mapped.memory == NULL) {

		ID3D11Resource *lpResource = xx->d3d11.resource;

#if defined(_DEBUG)
		{
			HRESULT error = IUnknown_QueryInterface(lpResource,&IID_ID3D11Resource,(void**)&lpResource);
			if FAILED(error) {
				__debugbreak();
			}
		}
#endif

		D3D11_MAPPED_SUBRESOURCE memory_d3d;
		HRESULT error = ID3D11DeviceContext_Map(LGI.d3d11.ctx,lpResource,0,D3D11_MAP_WRITE_DISCARD,0,&memory_d3d);
		if SUCCEEDED(error) {
			xx->mapped.stride = memory_d3d.RowPitch;
			xx->mapped.memory = memory_d3d.pData;
		} else {
			xx->mapped.stride = 0;
			xx->mapped.memory = 0;
		}
	}
	if (lpStride != NULL) {
		*lpStride = xx->mapped.stride;
	}
	return xx->mapped.memory;
}

lgi_API void lgi_updateBufferContents(lgi_Buffer *xx, void *source, size_t length) {
	void *target = lgi_borrowBufferContents(xx,NULL);
	CopyMemory(target,source,length);
	lgi_returnBufferContents(xx);
}

lgi_API void lgi_deleteBuffer(lgi_Buffer *xx) {
	lgi_closeHandle((lgi_Unknown)xx->d3d11.buffer);
	lgi__deallocate_memory(xx,NULL);
}

lgi_API lgi_Buffer *lgi_makeConstBuffer(unsigned int length, void *memory) {
	D3D11_BUFFER_DESC the_buffer_info;
	the_buffer_info.              Usage = D3D11_USAGE_DYNAMIC;
	the_buffer_info.     CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	the_buffer_info.          MiscFlags = 0;
	the_buffer_info.StructureByteStride = 0;
	the_buffer_info.          BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	the_buffer_info.          ByteWidth = (UINT)((length+15)/16*16);

	ID3D11Buffer *the_buffer;
	ID3D11Device_CreateBuffer(LGI.d3d11.dev,&the_buffer_info,NULL,&the_buffer);

	lgi_Buffer *buffer = lgi__allocate_memory(sizeof(lgi_Buffer),NULL);
	buffer->d3d11.buffer = the_buffer;
	buffer->d3d11.shader_target = lgi_Null;
	buffer->mapped.stride = 0;
	buffer->mapped.memory = 0;

	return buffer;
}

lgi_API lgi_Buffer *lgi_makeIndexBuffer(int index_size, int index_count) {

	D3D11_BUFFER_DESC the_buffer_info;
	the_buffer_info.              Usage = D3D11_USAGE_DYNAMIC;
	the_buffer_info.     CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	the_buffer_info.          MiscFlags = 0;
	the_buffer_info.StructureByteStride = 0;
	the_buffer_info.          BindFlags = D3D11_BIND_INDEX_BUFFER;
	the_buffer_info.          ByteWidth = (UINT)(index_size * index_count);

	ID3D11Buffer *the_buffer;
	ID3D11Device_CreateBuffer(LGI.d3d11.dev,&the_buffer_info,NULL,&the_buffer);

	lgi_Buffer *buffer = lgi__allocate_memory(sizeof(lgi_Buffer),NULL);
	buffer->d3d11.buffer = the_buffer;
	buffer->d3d11.shader_target = lgi_Null;
	buffer->mapped.stride = 0;
	buffer->mapped.memory = 0;

	return buffer;
}

lgi_API lgi_Buffer *lgi_makeVertexBuffer(int vertex_size, int vertex_count) {
	D3D11_BUFFER_DESC the_buffer_info;
	the_buffer_info.              Usage = D3D11_USAGE_DYNAMIC;
	the_buffer_info.     CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	the_buffer_info.          MiscFlags = 0;
	the_buffer_info.StructureByteStride = 0;
	the_buffer_info.          BindFlags = D3D11_BIND_VERTEX_BUFFER;
	the_buffer_info.          ByteWidth = (UINT)(vertex_size * vertex_count);

	ID3D11Buffer *the_buffer;
	ID3D11Device_CreateBuffer(LGI.d3d11.dev,&the_buffer_info,NULL,&the_buffer);

	lgi_Buffer *buffer = lgi__allocate_memory(sizeof(lgi_Buffer),NULL);
	buffer->d3d11.buffer = the_buffer;
	buffer->d3d11.shader_target = lgi_Null;
	buffer->mapped.stride = 0;
	buffer->mapped.memory = 0;
	return buffer;
}

lgi_API lgi_Buffer *lgi_makeStructuredBuffer(int struct_size, int struct_count) {

	D3D11_BUFFER_DESC the_buffer_info;
	the_buffer_info.              Usage=D3D11_USAGE_DYNAMIC;
	the_buffer_info.     CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
	the_buffer_info.          MiscFlags=D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	the_buffer_info.StructureByteStride=(UINT)(struct_size);
	the_buffer_info.          BindFlags=D3D11_BIND_SHADER_RESOURCE;
	the_buffer_info.          ByteWidth=(UINT)(struct_size * struct_count);

	ID3D11Buffer *the_buffer;
	ID3D11Device_CreateBuffer(LGI.d3d11.dev,&the_buffer_info,NULL,&the_buffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC D;
	D.             Format=DXGI_FORMAT_UNKNOWN;
	D.      ViewDimension=D3D11_SRV_DIMENSION_BUFFER;
	D.Buffer.FirstElement=0;
	D.Buffer. NumElements=(UINT)(struct_count);

	ID3D11ShaderResourceView *View;
	ID3D11Device_CreateShaderResourceView(LGI.d3d11.dev,(ID3D11Resource*)the_buffer,&D,&View);

	lgi_Buffer *buffer = lgi__allocate_memory(sizeof(lgi_Buffer),NULL);
	buffer->d3d11.buffer = the_buffer;
	buffer->d3d11.shader_target = View;
	buffer->mapped.stride = 0;
	buffer->mapped.memory = 0;

	return buffer;
}

lgi_API void lgi_clearTexture(lgi_Texture *xx, unsigned char bit) {
	int stride;
	void *memory = lgi_borrowTextureContents(xx,&stride);
	memset(memory,bit,xx->size_y*stride);
	lgi_returnTextureContents(xx);
}

lgi_API void lgi_updateTexture(lgi_Texture *xx, lgi_Image image) {
	lgi_ASSERT(image.format == xx->format);
	int stride;
	void *memory = lgi_borrowTextureContents(xx,&stride);
	// TODO: this is wrong!
	memcpy(memory,image.memory,image.size_y*image.stride);
	lgi_returnTextureContents(xx);
}

lgi_API void lgi_returnTextureContents(lgi_Texture *xx) {
	lgi_ASSERT(xx->mapped.memory != NULL);
	if (xx->mapped.memory != NULL) {
		ID3D11DeviceContext_Unmap(LGI.d3d11.ctx,xx->d3d11.resource,0);
		xx->mapped.memory = NULL;
	}
}

lgi_API void *lgi_borrowTextureContents(lgi_Texture *xx, int *stride) {
	lgi_ASSERT(xx->mapped.memory == NULL);
	if (xx->mapped.memory == NULL) {
		D3D11_MAPPED_SUBRESOURCE subres_d3d;
		HRESULT error = ID3D11DeviceContext_Map(LGI.d3d11.ctx,xx->d3d11.resource,0,D3D11_MAP_WRITE_DISCARD,0,&subres_d3d);
		if SUCCEEDED(error) {
			xx->mapped.stride = subres_d3d.RowPitch;
			xx->mapped.memory = subres_d3d.pData;
		} else {
			xx->mapped.stride = 0;
			xx->mapped.memory = 0;
		}
	}

	if (stride != NULL) {
		*stride = xx->mapped.stride;
	}
	return xx->mapped.memory;
}

lgi_API lgi_Texture_Config
lgi_makeTextureConfig(int size_x, int size_y, DXGI_FORMAT format, int stride
, void  *memory, int samples, int quality
, D3D11_USAGE memtype, int useflag, int memflag) {

	lgi_Texture_Config config;
	lgi__clear_typeof(&config);
	config.size_x     = size_x;
	config.size_y     = size_y;
	config.format     = format;
	config.stride     = stride;
	config.memory     = memory;
	config.samples    = samples;
	config.quality    = quality;
	config.memtype    = memtype;
	config.useflag    = useflag;
	config.memflag    = memflag;
	return config;
}

lgi_API lgi_Error lgi_initTexture(lgi_Texture *texture, lgi_Texture_Config *config) {
	lgi_Error error = lgi_Error_NONE;

	lgi_ASSERT(lgi.LINEAR_SAMPLER != NULL);

	if (config->d3d11.sampler == NULL) {
		config->d3d11.sampler = lgi.LINEAR_SAMPLER;
	}

	if (config->d3d11.texture_2d == NULL) {
		//
		// TODO:
		//
		lgi_ASSERT((config->size_x >= 1 && config->size_x <= 16384)
		|| lgi_logError("invalid size x %i", config->size_x));
		lgi_ASSERT((config->size_y >= 1 && config->size_y <= 16384)
		|| lgi_logError("invalid size y %i", config->size_y));

		D3D11_TEXTURE2D_DESC config_d3d;
		config_d3d.Width=config->size_x;
		config_d3d.Height=config->size_y;
		config_d3d.MipLevels=1;
		config_d3d.ArraySize=1;
		config_d3d.Format=config->format;
		config_d3d.SampleDesc.Count=1;
		config_d3d.SampleDesc.Quality=0;
		config_d3d.Usage=config->memtype;
		config_d3d.BindFlags=config->useflag;
		config_d3d.CPUAccessFlags=config->memflag;
		config_d3d.MiscFlags=0;

		if (config->memory != NULL) {

			D3D11_SUBRESOURCE_DATA data_config_d3d;
			lgi__clear_typeof(&data_config_d3d);
			data_config_d3d.    pSysMem=config->memory;
			data_config_d3d.SysMemPitch=config->stride;

			ID3D11Device_CreateTexture2D(LGI.d3d11.dev
			,	&config_d3d,&data_config_d3d,&config->d3d11.texture_2d);
		} else {
			ID3D11Device_CreateTexture2D(LGI.d3d11.dev
			,	&config_d3d,NULL,&config->d3d11.texture_2d);
		}

		if (config->d3d11.texture_2d == NULL) {
			error = lgi_Error_CREATE_TEXTURE;
			goto L_leave;
		}
	}

	if(config->d3d11.depth_target == NULL) {
		if((config->useflag & D3D11_BIND_DEPTH_STENCIL) != 0) {

			ID3D11Device_CreateDepthStencilView(LGI.d3d11.dev,
			config->d3d11.resource,NULL,&config->d3d11.depth_target);

			if(config->d3d11.depth_target == NULL) {
				error = lgi_Error_CREATE_TEXTURE;
				goto L_leave;
			}
		}
	}

	if (config->d3d11.color_target == NULL) {
		if ((config->useflag & D3D11_BIND_RENDER_TARGET) != 0) {

			D3D11_RENDER_TARGET_VIEW_DESC view_config_d3d;
			lgi__clear_typeof(&view_config_d3d);
			view_config_d3d.Format       =DXGI_FORMAT_UNKNOWN;
			view_config_d3d.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;

			ID3D11Device_CreateRenderTargetView(LGI.d3d11.dev
			,	config->d3d11.resource,&view_config_d3d,&config->d3d11.color_target);

			if (config->d3d11.color_target == NULL) {
				error = lgi_Error_CREATE_TEXTURE;
				goto L_leave;
			}
		}
	}

	if(config->d3d11.shader_target == NULL) {
		if((config->useflag & D3D11_BIND_SHADER_RESOURCE) != 0) {

			D3D11_SHADER_RESOURCE_VIEW_DESC texture_view_config_d3d;
			lgi__clear_typeof(&texture_view_config_d3d);
			texture_view_config_d3d.       Format=DXGI_FORMAT_UNKNOWN;
			texture_view_config_d3d.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
			texture_view_config_d3d.    Texture2D.MostDetailedMip=0;
			texture_view_config_d3d.    Texture2D.      MipLevels=1;

			ID3D11Device_CreateShaderResourceView(LGI.d3d11.dev
			,	config->d3d11.resource,&texture_view_config_d3d,&config->d3d11.shader_target);

			if(config->d3d11.shader_target == NULL) {
				error = lgi_Error_CREATE_TEXTURE;
				goto L_leave;
			}
		}
	}

	L_leave:
	texture->size_x=config->size_x;
	texture->size_y=config->size_y;
	texture->format=config->format;
	texture->samples=config->samples;
	texture->quality=config->quality;
	texture->d3d11.sampler=config->d3d11.sampler;
	texture->d3d11.resource=config->d3d11.resource;
	texture->d3d11.shader_target=config->d3d11.shader_target;
	texture->d3d11.color_target=config->d3d11.color_target;
	texture->d3d11.depth_target=config->d3d11.depth_target;
	texture->mapped.memory=0;
	texture->mapped.stride=0;

	lgi_ASSERT(texture->d3d11.sampler != NULL);
	lgi_ASSERT(texture->d3d11.resource != NULL);

	return error;
}

lgi_API void lgi_closeTexture(lgi_Texture *texture) {
	lgi_closeHandle((lgi_Unknown) texture->d3d11.shader_target);
	lgi_closeHandle((lgi_Unknown) texture->d3d11.resource);
}

lgi_API void lgi_deleteTexture(lgi_Texture *texture) {
	lgi_closeTexture(texture);
	lgi__deallocate_memory(texture,NULL);
}

lgi_API lgi_Texture *lgi_createTexture(lgi_Texture_Config *config) {
	lgi_Texture *result = lgi__allocate_typeof(lgi_Texture);
	lgi_Error error = lgi_initTexture(result,config);
	if (error != lgi_Error_NONE) {
		lgi_deleteTexture(result);
		lgi__deallocate_memory(result,NULL);
		result = NULL;
	}
	return result;
}

lgi_API void lgi_copyTexture(lgi_Texture *dst, lgi_Texture *src) {
	if (dst->d3d11.resource != src->d3d11.resource) {
		if (src->samples <= 1) {
			ID3D11DeviceContext_CopyResource(LGI.d3d11.ctx,dst->d3d11.resource,src->d3d11.resource);
		} else {
			ID3D11DeviceContext_ResolveSubresource(LGI.d3d11.ctx,dst->d3d11.resource,0,src->d3d11.resource,0,dst->format);
		}
	}
}


lgi_API lgi_Texture *lgi_makeTexture(int size_x, int size_y, DXGI_FORMAT format, int stride, void *memory) {
	lgi_Texture_Config config = lgi_makeTextureConfig(size_x,size_y,format,stride,memory,1,0,D3D11_USAGE_DYNAMIC,D3D11_BIND_SHADER_RESOURCE,D3D11_CPU_ACCESS_WRITE);
	return lgi_createTexture(&config);
}


lgi_API lgi_Texture *lgi_loadTexture(char const *fileName) {
	return lgi_uploadImage(lgi_loadImage(fileName));
}


lgi_API lgi_Texture *lgi_makeDepthTarget(int size_x, int size_y, int format) {
	lgi_Texture_Config config = lgi_makeTextureConfig(size_x,size_y,format,0,NULL,1,0,D3D11_USAGE_DEFAULT,D3D11_BIND_DEPTH_STENCIL,0);
	return lgi_createTexture(&config);
}

lgi_API lgi_Texture *lgi_makeColorTarget(int size_x, int size_y, int format, int samples, int quality) {
	lgi_Texture_Config config = lgi_makeTextureConfig(size_x,size_y,format,0,0,samples,quality,D3D11_USAGE_DEFAULT,D3D11_BIND_RENDER_TARGET,0);
	return lgi_createTexture(&config);
}

//
// Shader API:
//

lgi_API lgi_Shader *lgi_loadShader(int flags, char const *label, char const *entry, char const *fileName) {

	__int32 length = 0;
	void *memory = lgi_loadFileContents(fileName,&length);
	if (memory != NULL) {

		lgi_Shader_Config config;
		lgi__clear_typeof(&config);

		config.flags=flags;
		config.label=label;
		config.source.compile.memory=memory;
		config.source.compile.length=length;
		config.source.compile.entry=entry;

		lgi_Shader *shader = lgi__allocate_typeof(lgi_Shader);
		lgi_initShader(shader,&config);

		lgi_unloadFileContents(memory);

		return shader;
	}

	return NULL;
}

lgi_API lgi_Shader *lgi_buildPixelShader(int flags, char const *label, size_t hlsl_length, void *hlsl_memory) {
	return lgi_buildShader(lgi_PIXEL_SHADER_TYPE|flags,label,hlsl_length,hlsl_memory);
}

lgi_API lgi_Shader *lgi_buildVertexShader(int flags, char const *label, size_t hlsl_length, void *hlsl_memory) {
	return lgi_buildShader(lgi_VERTEX_SHADER_TYPE|flags,label,hlsl_length,hlsl_memory);
}

lgi_API lgi_Shader *lgi_buildShader(int flags, char const *label, size_t hlsl_length, void *hlsl_memory) {
	lgi_Shader_Config config;
	lgi__clear_typeof(&config);

	config.flags = flags;
	config.label = label;
	config.source.bytecode.length = hlsl_length;
	config.source.bytecode.memory = hlsl_memory;

	lgi_Shader *shader = lgi__allocate_typeof(lgi_Shader);
	lgi_initShader(shader,&config);
	return shader;
}

lgi_API char const * lgi__getShaderEntryName(int shader_flags) {
	if(shader_flags & lgi_VERTEX_SHADER_TYPE) {
		return "MainVS";
	} else
	if(shader_flags & lgi_PIXEL_SHADER_TYPE) {
		return "MainPS";
	} else
	if(shader_flags & lgi_COMPUTE_SHADER_TYPE) {
		return "MainCS";
	}
	return NULL;
}

lgi_API char const *lgi__getShaderModelName(int shader_flags) {
	//
	// TODO:
	//
	if (shader_flags & lgi_VERTEX_SHADER_TYPE) {
		return "vs_5_0";
	} else
	if (shader_flags & lgi_PIXEL_SHADER_TYPE) {
		return "ps_5_0";
	} else
	if (shader_flags & lgi_COMPUTE_SHADER_TYPE) {
		return "cs_5_0";
	}
	return NULL;
}

lgi_API lgi_Error lgi__compileShader(lgi_Shader *shader, lgi_Shader_Config *config) {
	ID3DBlob *bytecode_blob_d3d = NULL;
	ID3DBlob *messages_blob_d3d = NULL;

	if (config->source.compile.debug_label == NULL) {
		config->source.compile.debug_label = config->label;
	}

	char const *model = config->source.compile.model;
	if (model == NULL) {
		config->source.compile.model = model = lgi__getShaderModelName(config->flags);
	}
	char const *entry = config->source.compile.entry;
	if (entry == NULL) {
		config->source.compile.entry = entry = lgi__getShaderEntryName(config->flags);
	}

	void *memory = config->source.compile.memory;
	size_t length = config->source.compile.length;
	char const *debug_label = config->source.compile.debug_label;
	lgi_ASSERT(entry != lgi_Null);
	lgi_ASSERT(model != lgi_Null);
	lgi_ASSERT(memory != lgi_Null);
	lgi_ASSERT(length != 0);

	HRESULT error = D3DCompile(memory,length,debug_label,0,0,entry,model,lgi_DEFAULT_SHADER_BUILD_FLAGS,0,&bytecode_blob_d3d,&messages_blob_d3d);
	if SUCCEEDED(error) {
		if(messages_blob_d3d != lgi_Null) {
			lgi_logError("Shader Compilation Warning:\n%s\n",(char*)(messages_blob_d3d->lpVtbl->GetBufferPointer(messages_blob_d3d)));
		}
		config->source.bytecode.memory=bytecode_blob_d3d->lpVtbl->GetBufferPointer(bytecode_blob_d3d);
		config->source.bytecode.length=bytecode_blob_d3d->lpVtbl->   GetBufferSize(bytecode_blob_d3d);

		if(bytecode_blob_d3d != lgi_Null) {
			bytecode_blob_d3d->lpVtbl->Release(bytecode_blob_d3d);
		}
		if(messages_blob_d3d != NULL) {
			messages_blob_d3d->lpVtbl->Release(messages_blob_d3d);
		}
		return lgi_Error_NONE;
	} else {
		lgi_logError("Shader Compilation Error:\n%s\n",(char*)(messages_blob_d3d->lpVtbl->GetBufferPointer(messages_blob_d3d)));
		return lgi_Error_SHADER_COMPILATION;
	}
}

lgi_API lgi_Error lgi__makeShaderInputLayout(lgi_Shader *shader, lgi_Shader_Config *config) {
	ID3DBlob *sig_blob_d3d = NULL;
	ID3D11ShaderReflection *reflect_d3d = NULL;

	lgi_Error result = lgi_Error_NONE;

	void *memory = config->source.bytecode.memory;
	size_t length = config->source.bytecode.length;

	if (config->layout.d3d11.layout == NULL) {
		if (config->flags & lgi_PIXEL_SHADER_TYPE) {
			lgi_logInfo("Skipped Input Layout Creation For Pixel Shader");
		}
		if(config->layout.attr_count == 0) {
	      /* ensure signature blob is present otherwise early out, in my experience
	        even with the /Qstrip_reflect /Qstrip_priv you still get the data you need,
	        either way I think is good to let the user know that they could try
	        and remove those flags in case it fails */

			HRESULT error = D3DGetBlobPart(memory,length,D3D_BLOB_INPUT_SIGNATURE_BLOB,0,&sig_blob_d3d);
			if FAILED(error) {
				result = lgi_Error_SHADER_COMPILATION;
				goto L_leave;
			}
			error = D3DReflect(memory,length,&IID_ID3D11ShaderReflection,(void**)&reflect_d3d);
			if FAILED(error) {
				result = lgi_Error_SHADER_COMPILATION;
				goto L_leave;
			}

			D3D11_SHADER_DESC shader_info_d3d;
			error = reflect_d3d->lpVtbl->GetDesc(reflect_d3d,&shader_info_d3d);
			lgi_ASSERT(SUCCEEDED(error));

			config->layout.attr_count = shader_info_d3d.InputParameters;

			int index;
			for (index = 0; index < config->layout.attr_count; index += 1) {
				D3D11_SIGNATURE_PARAMETER_DESC param_info_d3d;
				error = reflect_d3d->lpVtbl->GetInputParameterDesc(reflect_d3d,index,&param_info_d3d);
				lgi_ASSERT(SUCCEEDED(error));

				DXGI_FORMAT Format = 0;
				//
				// TODO: Finish this UP!
				//
				if (param_info_d3d.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
					switch(param_info_d3d.Mask) {
						case 0b1000:
						case 0b0100:
						case 0b0010:
						case 0b0001: Format=DXGI_FORMAT_R32_FLOAT;          break;
						case 0b1100:
						case 0b0011: Format=DXGI_FORMAT_R32G32_FLOAT;       break;
						case 0b0111: Format=0;                              break;
						case 0b1111: Format=DXGI_FORMAT_R32G32B32A32_FLOAT; break;
						default: {
							result = lgi_Error_SHADER_COMPILATION;
							lgi_ASSERT(!"not implemented");
						} break;
					}
				} else {
					result = lgi_Error_SHADER_COMPILATION;
					lgi_ASSERT(!"not implemented");
				}

				D3D11_INPUT_ELEMENT_DESC *elem_config_d3d=&config->layout.attr_array[index];
				elem_config_d3d->Format=Format;
				elem_config_d3d->SemanticName=param_info_d3d.SemanticName;
				elem_config_d3d->SemanticIndex=param_info_d3d.SemanticIndex;
				elem_config_d3d->AlignedByteOffset=D3D11_APPEND_ALIGNED_ELEMENT;
				elem_config_d3d->InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
				elem_config_d3d->InputSlot=0;
				elem_config_d3d->InstanceDataStepRate=0;
			}
		}

		int attr_count = config->layout.attr_count;
		D3D11_INPUT_ELEMENT_DESC *attr_array = config->layout.attr_array;
		HRESULT error = ID3D11Device_CreateInputLayout(LGI.d3d11.dev,attr_array,attr_count,memory,length,&config->layout.d3d11.layout);
		if FAILED(error) {
			result = lgi_Error_SHADER_COMPILATION;
			lgi_logError("Failed to Create Input Layout");
			goto L_leave;
		}
	}

	L_leave:
	if(sig_blob_d3d != NULL) {
		sig_blob_d3d->lpVtbl->Release(sig_blob_d3d);
	}
	if(reflect_d3d != NULL) {
		reflect_d3d->lpVtbl->Release(reflect_d3d);
	}

	return result;
}

lgi_API lgi_Error lgi_initShader(lgi_Shader *shader, lgi_Shader_Config *config) {
	lgi__clear_typeof(shader);


	ID3D11DeviceChild *shader_d3d = NULL;

	lgi_Error result = lgi_Error_NONE;
	if (config->source.bytecode.memory == lgi_Null) {
		result = lgi__compileShader(shader,config);
		if lgi_Failed(result) {
			goto L_leave;
		}
	}

	if (config->layout.d3d11.layout == lgi_Null) {
		result = lgi__makeShaderInputLayout(shader,config);
		if lgi_Failed(result) {
			goto L_leave;
		}
	}

	void *memory = config->source.bytecode.memory;
	size_t length = config->source.bytecode.length;

	if (config->flags & lgi_VERTEX_SHADER_TYPE) {
		HRESULT error = ID3D11Device_CreateVertexShader(LGI.d3d11.dev,memory,length,NULL,(ID3D11VertexShader**)(&shader_d3d));
		if FAILED(error) {
			lgi_logError("Failed to Create Vertex Shader");
			goto L_leave;
		}
	} else
	if (config->flags & lgi_PIXEL_SHADER_TYPE) {
		HRESULT error = ID3D11Device_CreatePixelShader(LGI.d3d11.dev,memory,length,NULL,(ID3D11PixelShader**)(&shader_d3d));
		if FAILED(error) {
			lgi_logError("Failed to Create Pixel Shader");
			goto L_leave;
		}
	} else
	if (config->flags & lgi_COMPUTE_SHADER_TYPE) {
		HRESULT error = ID3D11Device_CreateComputeShader(LGI.d3d11.dev,memory,length,NULL,(ID3D11ComputeShader**)(&shader_d3d));
		if FAILED(error) {
			lgi_logError("Failed to Create Compute Shader");
			goto L_leave;
		}
	} else {
		lgi_ASSERT(!"error");
	}

	L_leave:
	shader->label = config->label;
	shader->flags = config->flags;
	shader->d3d11.unknown = shader_d3d;
	shader->d3d11.layout  = config->layout.d3d11.layout;

	return result;
}
