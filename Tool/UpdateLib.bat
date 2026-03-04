xcopy			/y		..\CoreLib\Bin\Debug\CoreLib.lib			.\Lib\Debug\
xcopy			/y		..\CoreLib\Bin\Release\CoreLib.lib			.\Lib\Release\

xcopy			/y		..\RenderLib\Bin\Debug\RenderLib.lib		.\Lib\Debug\
xcopy			/y		..\RenderLib\Bin\Release\RenderLib.lib		.\Lib\Release\

xcopy			/y		..\GameEngineLib\Bin\Debug\GameEngineLib.lib		.\Lib\Debug\
xcopy			/y		..\GameEngineLib\Bin\Release\GameEngineLib.lib		.\Lib\Release\

xcopy			/y		..\ServerCoreLib\Bin\Debug\ServerCoreLib.lib			.\Lib\Debug\
xcopy			/y		..\ServerCoreLib\Bin\Release\ServerCoreLib.lib			.\Lib\Release\

xcopy			/y		..\CoreLib\*.h									.\Inc\
xcopy			/y		..\RenderLib\*.h								.\Inc\
zxcopy			/y		..\ServerCoreLib\*.h							.\Inc\
xcopy			/y		..\GameEngineLib\*.h							.\Inc\
