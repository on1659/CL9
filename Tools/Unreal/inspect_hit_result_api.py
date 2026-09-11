import unreal
for cls in [unreal.GameplayStatics,unreal.SystemLibrary,unreal.HitResult]:
 print(cls.__name__,[n for n in dir(cls) if 'hit' in n.lower() or 'impact' in n.lower()])
 if hasattr(cls,'break_hit_result'):print(cls.break_hit_result.__doc__)
