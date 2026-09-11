import unreal
for cls in [unreal.GameplayStatics,unreal.World,unreal.SystemLibrary]:
    print(cls.__name__,[n for n in dir(cls) if 'spawn' in n or 'break_hit' in n])
for name in ['line_trace_single_by_profile','capsule_trace_single_by_profile']:
    print(getattr(unreal.SystemLibrary,name).__doc__)
print(unreal.HitResult.__doc__)
