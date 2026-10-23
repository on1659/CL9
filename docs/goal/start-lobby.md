# goal: start-lobby

**Status**: implemented and verified as a solo host 2026-10-05 (run `start-lobby`, REPORTED, uncommitted) - gates B0/B1/B2 and G-FINAL pass, standalone boot lands on the Title; everything that needs a second player is unverified - see "Results" at the end. Spec settled 2026-10-05 by `/autogoal` (HEAVY). Morning summary: `.claude/runs/control-feel/morning.md`. Sibling spec and the shared execution plan: [control-feel.md](control-feel.md).

## One-line Goal
Boot into a start level with a title menu, host a stage from it, wait in a walkable waiting room (the stage's Lobby phase at the truck), depart with a lever, and — when everyone is dead (solo: when I die) — see a game-over screen that leads back to the start level.

## Background / Motivation
The owner: "make a start level; like Lethal, you should get into the game starting from a waiting room; UI work is needed along the way; it must go all the way to: when I die, the game ends" (2026-10-05, asked to decide without questions).

What recon found:
- CL9 runs one state machine `ECL9NightPhase {Lobby, Night, Dawn}` on a host `ACL9GameMode : AGameModeBase`; a whole run lives in one stage level; there is no travel or session code; the night can only be started by the cheat `cl9.night`; a wipe runs `EndNight` inside the damage call stack and simply continues with the next day; there is no game-over screen and no way back to a menu.
- Lethal Company's waiting room is the ship itself — the same scene as the run; the start lever is a short deliberate interaction, host-only for the first start. R.E.P.O. sends a wiped crew to a game-over screen and back to the lobby.
- `docs/systems/session.md` (unimplemented pre-doc) and `docs/goal/steam-release.md` W3 (awaiting approval) both exist; W3's "one title level + one game level, truck as the waiting room, departure = StartNight" is the structure adopted here. Steam transport is not part of tonight.
- After any hard travel the UI manager would be dead (it only re-bootstraps when the main widget pointer is invalid, but the widget survives the world) — found by the panel, fixed here.

## Flow
```
boot / editor: Lvl_Start  (empty level, native ACL9MenuGameMode, no pawn, HUD collapsed)
  Title (root screen):  [방 만들기] -> stage list (sites with a Level) -> HostGame(alias)   [설정]   [종료]
      -> OpenLevel(<stage>, listen in non-Shipping)
stage level, Phase Lobby  == the waiting room (walkable truck yard; orders, shop, truck pad as today)
  depart lever next to the truck pad (runtime-spawned): press -> armed 5 s -> press again -> StartNight
      (offer still pending -> the press re-opens the order screen instead)
Night ... individual death -> spectate (exists)
  everyone dead + CL9Rules.bWipeEndsRun  ->  after SpectateDelay: night torn down, Phase held at Dawn,
      GameState.RunOver {bActive, Day, BestDay} replicated
  -> GameOver (root screen, derived from state on every machine)
        host: [시작 화면으로]      client: [나가기] + "호스트가 시작 화면으로 돌아가면 함께 나갑니다"
        -> UCL9GameInstance::LeaveGame() -> Lvl_Start -> Title
```

## In-scope (must-land)
- RUN B ui: UCL9UIManager stale-widget re-bootstrap + OpenRootScreen/IsRootScreen/root no-op close; CL9UINames::GameOver; self-building UCL9TitleWidget (stage list, settings, quit) and UCL9GameOverWidget; UCL9HUDWidget DeriveRunOverScreen + self-hide until wired; strings from No 208; docs/systems/ui.md and ui_system.md
- RUN B gameplay: UCL9GameInstance HostGame/LeaveGame/JoinGame/CancelJoin/IsJoining/IsJoinAvailable + Config StartLevel; ACL9MenuGameMode; ACL9PlayerController IsInMenuWorld, Title opening at BeginPlay, DebugPanel guard; ECL9DepartResult, FCL9RunOver; ACL9GameState RunOver replication; ACL9GameMode RequestDepart, SpawnDepartLever, run-over timer + FinishRunOver + EndNight guard, RecordBestDay returning the record; ACL9DepartLever (including the re-open of OrderSelect when an offer is pending); UCL9RulesDef bWipeEndsRun/DepartArmTime/DepartLeverMesh/DepartLeverOffset; cheats cl9.host, cl9.leave, cl9.join, cl9.depart, cl9.lever; docs/systems/session.md, cheats.md, docs/plan/data/rules.md
- RUN B lead: StartLevel config line while the editor is closed; build #2; P0 log scan; Lvl_Start with the native GameMode override; two shell WBPs; UITable +2 rows with old rows verified; CL9Rules DepartLeverMesh cell; Gate B1 (flag off); flip bWipeEndsRun; Gate B2 (full loop); GameDefaultMap line only if B2 passed; G-FINAL; decisions.md entry marking the wipe reversal as owner-instructed; morning.md
- Both runs: CL9ReplicationCheck automation test passes with the new replicated fields (GameState RunOver, lever ArmedUntilServerTime)

## Stretch
- S5 (ui, Run B, after must-land): OrderSelect auto-opens once per offer so the lobby is walkable before orders are picked
- S6 (ui, Run B, after S5): Title join section - address box default 127.0.0.1, [접속], '접속 중…' with [취소] - on top of the must-land JoinGame/CancelJoin/IsJoining API; otherwise the Join button is not shown
- S7 (lead, only after G-FINAL is green and before T+4:40): dressed Lvl_Start backdrop (20 min cap); one 2-client PIE with p.NetShowCorrections 1 at t.MaxFPS 60 and 144; standalone -game boot check of GameDefaultMap

## Out-of-scope
- Separate lobby level, ServerTravel, OnlineSubsystemSteam
- ACL9MenuPlayerController, BP_CL9MenuGameMode, BP_CL9DepartLever, DepartLeverClass on GameMode BPs, a new Blender lever mesh, hand placement in the four stage umaps
- In-place RestartRun, cl9.restart, save deletion on run over, a reason field on RunOver, a new ECL9NightPhase value
- PreLogin join gate, GameSession MaxPlayers 4, mid-night join rejection, SlotIndex and roster, ECL9SessionNotice and network/travel failure handlers, Client_ReturnToStart, Client_DepartRefused, Logout wipe re-check, custom join timeout
- UITable rows Host, Join, Loading, Notice; RosterBox in WBP_HUD; UCL9PauseWidget and the WBP_Pause leave button; any PauseInput edit; cl9.wipe (cl9.damage on a solo host is a wipe)
- Rewrite of docs/testing/pie-scenarios.md H3 and the steam-release/session.md reconciliation (docs pass after the owner has seen the loop)
- Run C

## Design Decisions (panel-settled — do not relitigate during implementation)
- **L1 Waiting room** — Confirmed: the waiting room is the stage level's Lobby phase at the truck. One menu level, one hard travel (OpenLevel with listen), AGameModeBase kept, no lobby level, no ServerTravel.
  - Rationale: Only structure verifiable in PIE tonight, needs no state carry-over, matches steam-release W3 and the existing phase machine.
  - Rejected: Separate walkable lobby level with seamless travel.
- **L2 Menu level, game mode, controller** — Lvl_Start is an empty level whose World Settings GameMode override is the native, non-abstract ACL9MenuGameMode (class path /Script/CL9.CL9MenuGameMode): no pawn, no HUD class, HandleStartingNewPlayer does nothing, InitGame copies PlayerControllerClass from the CDO of UGameMapsSettings::GetGlobalDefaultGameMode(). BP_CL9PlayerController is reused; ACL9PlayerController::BeginPlay opens the menu game mode's TitleUIRow as a root screen after InitForPlayer, and DebugPanelInput returns when IsInMenuWorld(). No IMC guard is needed because an open Screen already detaches IMC_Gameplay.
  - Rationale: Zero new BP assets, keeps UI bootstrap, IMC registration for the settings key list and audio apply. Opaque Title makes a dressed backdrop optional.
  - Rejected: Flow seat's BP_CL9MenuGameMode asset and abstract class; draft's ACL9MenuPlayerController; a dressed backdrop level tonight.
- **L3 UI manager after hard travel, root screens** — InitForPlayer treats the UI as stale when !IsValid(MainWidget) || OwningPlayer.Get() != PlayerController || !MainWidget->IsInViewport(); on stale it removes every open widget and the main widget from parent, clears OpenWidgets/ScreenHistory/OpenScreenName/RootScreens, recreates, and logs 'UIManager: bootstrapped for <PC> in <World>'. New OpenRootScreen(UIName) opens with history reset and marks the name as root; CloseWidget on a root screen is a no-op (so M, [X] and CloseSelf cannot close Title or GameOver; another Screen opened over it still replaces it and history restores it). PauseInput is not edited.
  - Rationale: Verified blocker (CL9UIManager.cpp:33-44 with UserWidget.cpp:2753 and GameViewportSubsystem.cpp:301-322). Putting the root rule inside the manager removes the cross-role edit of PauseInput.
  - Rejected: Flow seat's PauseInput rewrite with a root list; menu controller that does not bind IA_Pause.
- **L4 Transport and leaving** — UCL9GameInstance::HostGame(SiteAlias) opens the site row's Level (appending 'listen' only in !UE_BUILD_SHIPPING). LeaveGame() = OpenLevel of the config StartLevel ([/Script/CL9.CL9GameInstance] StartLevel=/Game/Maps/Lvl_Start in Config/DefaultGame.ini), falling back to UGameMapsSettings::GetGameDefaultMap() when empty; host and client call the same function (the host's non-listen travel drops clients). JoinGame(Address)/CancelJoin()/IsJoining() are implemented (non-Shipping) as must-land API so the Title join UI, which is stretch, can never break the build; cl9.join is must-land.
  - Rationale: OpenLevel is the one travel mechanism the gate already has to prove for hosting, and the config line keeps asset paths out of C++ and makes leaving independent of the GameDefaultMap flip. Session calls stay behind the GameInstance for the later Steam swap.
  - Rejected: Flow seat's ReturnToMainMenu (depends on flipping GameDefaultMap early, different engine path from hosting) and ECL9SessionNotice/failure handlers tonight; listening in Shipping; OnlineSubsystemSteam tonight.
- **L5 Destination choice** — Inline stage list inside the Title screen built from registry sites whose Level is set; clicking a stage calls HostGame(alias). Only henhouse is verified tonight.
  - Rationale: Data-driven, no extra screen or asset.
  - Rejected: Separate Host screen row.
- **L6 Depart lever** — Native non-abstract ACL9DepartLever : ACL9Fixture, ICL9Interactable (Source/CL9/Items), spawned at runtime by ACL9GameMode::StartPlay next to the truck pad from UCL9RulesDef::DepartLeverOffset; mesh from the soft pointer UCL9RulesDef::DepartLeverMesh loaded locally on every machine (null = invisible but functional); query-only collision plus a 60x60x80 box so it is focusable and never blocks the capsule. Two-step press, no hold: CanInteract = Who && Phase == Lobby && !IsRunOver. Interact (authority; ignored unless Who's controller IsLocalController()): offer pending -> Controller->Client_OpenScreen(CL9UINames::OrderSelect); not armed -> ArmedUntilServerTime = now + Rules->DepartArmTime (replicated float, no timer, no OnRep); armed -> GameMode->RequestDepart(Controller) and disarm. Prompts: host Prompt.DepartOrders / Prompt.Depart / Prompt.DepartConfirm, client Prompt.DepartHostOnly / Prompt.DepartSoon. RequestDepart checks host-local requester, Phase == Lobby, not run-over, OfferedOrders empty, no pending night start, then calls StartNight. cl9.night stays; cl9.depart and cl9.lever are the automation handles. Lead sets the mesh cell to /Game/Maps/Henhouse/Yard/SignExit/SM_Henhouse_Sign_Exit_A.
  - Rationale: The interaction path is press-only and prompts require CanInteract; progress rulings belong in the GameMode (conventions section 6); runtime spawn avoids saving four already-dirty stage umaps; the Blender static-mesh pipeline is stale on this machine. Verified StartNight itself only checks Phase == Lobby (CL9GameMode.cpp:209), so 'orders accepted' has to be the lever's own precondition.
  - Rejected: Flow seat's abstract class + BP_CL9DepartLever + DepartLeverClass on two GameMode BPs; draft's hand-placed BP and new Blender mesh; hold-to-depart; 3 s arm window (5 s costs nothing and auto-disarms).
- **L7 Run over** — Behind UCL9RulesDef::bWipeEndsRun (header default false = today's EndNight path; the lead flips the CL9Rules cell to true before Gate B2 and back if B2 fails). When true, the wipe branch of HandlePlayerDeath (Night only) starts RunOverHandle = SetTimer(FinishRunOver, max(Rules->SpectateDelay, 0.1)) if not already running and returns; EndNight returns early while RunOverHandle is valid. FinishRunOver re-checks Phase == Night and no living pawn, then: ClearTimer(NightExpiryHandle), CancelNightWork(), ClearNightPopulation(), GameState->ClearNightClock(), BestDay = RecordBestDay(CurrentDay) (now returns the record), GameState->SetPhase(Dawn), GameState->SetRunOver(CurrentDay, BestDay). It must NOT call AdvanceDay, ExpireOrders, LoseEquipmentLeftBehind, SetSiteMap, ReviveDead, OpenDawnScreen, Settle, SaveRun, OfferOrders or delete the save. Phase is held at Dawn; the only exit is leaving the level. Settlement failure keeps its current in-place restart.
  - Rationale: The wipe currently runs EndNight inside the damage stack (CL9GameMode.cpp:393-401). The SpectateDelay beat keeps the death cam; holding at Dawn reuses a transition every subscriber already handles and makes every Lobby-only gate, StartNight and EndNight refuse with zero new checks. The flag keeps the default dev flow intact if the GameOver asset step fails. The header default is deliberately false tonight; after owner sign-off it should be set to true to honour the header-default lesson.
  - Rejected: Flow seat's unconditional reversal with RestartRun in place and DeleteGameInSlot (an 8-step in-place reset and loss of the dev save for no visible benefit - re-hosting from Title gives a clean day 1 for free); scope seat's next-tick teardown (deletes the monster out of the death cam); a new ECL9NightPhase value.
- **L8 GameOver screen** — State-derived in the HUD tick like OrderSelect: UCL9HUDWidget::DeriveRunOverScreen opens CL9UINames::GameOver as a root screen when GameState->IsRunOver() and the open screen is not GameOver/Settings/DebugPanel/Pause; DeriveOrderScreen is skipped while run-over. UCL9GameOverWidget shows day reached and best day (replicated FCL9RunOver) and one button calling UCL9GameInstance::LeaveGame(): label GameOver.ToStart on the host, GameOver.Leave plus the GameOver.WaitHost line on a client. Dark backdrop (0,0,0,0.85).
  - Rationale: Tick derivation survives Spectate replacing the screen and late replication, needs no RPC and no PlayerController hook, and is gated on IsRunOver which is false unless the flag is on.
  - Rejected: Scope seat's PlayerController edge hook plus a Spectate-opener guard (two more edits in a shared file); two host buttons.
- **L9 Join gate, capacity, roster, notices** — Cut tonight: PreLogin gate, GameSession MaxPlayers 4, SlotIndex, roster, ECL9SessionNotice and failure handlers, Client_ReturnToStart, Logout wipe re-check. Today's behaviour stays (16 players, mid-night join, engine fallback on host loss) and is listed first under 'needs a 2-player session' in the morning report.
  - Rationale: None of it can be exercised by one host, the mid-night-join rule is an open gate in steam-release.md, and conventions section 6 requires host + client verification for network changes.
  - Rejected: Flow seat's D7/D8 set and two-instance PIE recipe as tonight's gate (not an existing recipe on this machine).
- **L10 UI set and construction** — Two UITable rows only: 'Title' and 'GameOver' (both Screen). UCL9TitleWidget and UCL9GameOverWidget are C++ UCL9Widget subclasses that build their whole tree themselves when WidgetTree->RootWidget is null (LOCTABLE text, WidgetTree->ConstructWidget buttons, no required BindWidget); the WBPs are empty shells (create with the C++ parent, compile, save). If shell creation fails the row points at the native class. Title: opaque backdrop, name, [방 만들기] -> inline stage list, [참가하기] only if the join UI stretch landed, [설정] -> existing Settings screen, [종료]. UCL9HUDWidget remembers its designer visibility in NativeConstruct, collapses itself, and restores it when TryWire succeeds, so it never shows in Lvl_Start. WBP_Pause, WBP_HUD, WBP_Main, IA/IMC assets and BP_CL9PlayerController are not edited.
  - Rationale: Three MCP calls per screen instead of a designed WBP, no BindWidget compile modal risk; the single TryWire success point (CL9HUDWidget.cpp:158-185) makes the self-hide safe; Settings opened from Title would otherwise show the unwired HUD behind it.
  - Rejected: Draft's seven surfaces; flow seat's Notice row, RosterBox and UCL9PauseWidget reparent; leaving the unwired HUD visible in the menu.
- **L11 Walkable lobby before orders (stretch S5)** — Stretch for the ui agent in Run B: OrderSelect auto-opens once per offer (bool bOrderScreenAutoOpened set when the HUD opens it, reset in HandleOrdersChanged); a user close sticks. The gameplay half that makes this safe - the lever re-opening OrderSelect for the host while an offer is pending - is must-land.
  - Rationale: Verified the HUD re-opens OrderSelect every tick while an offer exists (CL9HUDWidget.cpp:903-906), which freezes joining clients until the host picks, but the solo host loop works without the fix. Splitting it this way means a half-landed stretch can never leave the host unable to reach the order screen.
  - Rejected: Flow seat's blocker rating for tonight; leaving the lever with no path back to the order screen.
- **L12 GameDefaultMap** — GameDefaultMap in Config/DefaultEngine.ini is changed to /Game/Maps/Lvl_Start as the very last step and only if Gate B2 passed; EditorStartupMap stays Lvl_Flat. Until then the loop is reachable from the editor by opening Lvl_Start, because leaving uses StartLevel, not GameDefaultMap.
  - Rationale: Every failed join and host loss lands on GameDefaultMap; flipping it before the level is proven risks a blocking dialog or a stage game mode behind the menu.
  - Rejected: Flow seat's flip at the end of step 1.

## Parameters
| Name | Value | Basis |
|---|---|---|
| `UCL9RulesDef::bWipeEndsRun` | header default false; CL9Rules cell set true by the lead before Gate B2, back to false if B2 fails | Safe-failure switch for the reversal of 'wipe -> immediate EndNight' |
| `Run-over beat` | max(Rules->SpectateDelay, 0.1) s (SpectateDelay header default 3 s) | CL9RulesDef.h:55; keeps the death cam before teardown |
| `UCL9RulesDef::DepartArmTime` | 5.0 s | Window between the arming press and the confirming press |
| `UCL9RulesDef::DepartLeverOffset (FTransform, relative to the pad's +X face centre at pad base, in pad space)` | Location (40, 150, 130) cm, Rotation yaw 90 deg, Scale 1 | Same free side as WakeSpot (CL9GameMode.cpp:518-521: +X extent + 99 cm, Y +-60), 108 cm from the spawn spot, inside PickRadius 120 |
| `UCL9RulesDef::DepartLeverMesh` | header default null; lead sets the cell to /Game/Maps/Henhouse/Yard/SignExit/SM_Henhouse_Sign_Exit_A | Existing exit pictogram mesh; null = invisible but functional |
| `Start level pointer` | Config/DefaultGame.ini: [/Script/CL9.CL9GameInstance] StartLevel=/Game/Maps/Lvl_Start | UPROPERTY(Config) FSoftObjectPath on UCL9GameInstance (UCLASS gets Config=Game); written while the editor is closed for the Run B build |
| `Listen port / join address` | 7777 (URL default) / 127.0.0.1 | OpenLevel builds an absolute URL with the config default port |
| `String rows (ko column only)` | Run A No 207 Settings.CameraMotion = 화면 움직임. Run B from No 208: Title.Name = CL9; Title.Host = 방 만들기; Title.PickStage = 어디로 갈까요?; Title.Join = 참가하기; Title.Address = 주소; Title.Connect = 접속; Title.Connecting = 접속 중…; Title.Cancel = 취소; Title.Back = 뒤로; Title.Settings = 설정; Title.Quit = 종료; GameOver.Title = 게임 오버; GameOver.Day = {0}일차에서 끝났습니다; GameOver.Best = 최고 기록: {0}일차; GameOver.ToStart = 시작 화면으로; GameOver.Leave = 나가기; GameOver.WaitHost = 호스트가 시작 화면으로 돌아가면 함께 나갑니다; Prompt.Depart = 출발 준비; Prompt.DepartConfirm = 한 번 더 눌러 출발; Prompt.DepartOrders = 주문부터 고르기; Prompt.DepartHostOnly = 호스트만 출발할 수 있습니다; Prompt.DepartSoon = 곧 출발합니다 | Next free No is 207; Prompt.* rows follow the key-format convention of the existing Prompt.* rows (the ui agent matches Prompt.TruckPanelShop); ui runs sync_strings.py before reporting |

## Interfaces (implement verbatim — gameplay and ui code against these in parallel)
- **UCL9GameInstance session API** — owner: gameplay provides (Run B); ui Title/GameOver widgets consume — `C:/Work/CL9/Source/CL9/GameMode/CL9GameInstance.h/.cpp`
  - `UCLASS(abstract, Config=Game).  UFUNCTION(BlueprintCallable, Category="CL9|Session") bool HostGame(FName SiteAlias);  UFUNCTION(BlueprintCallable, Category="CL9|Session") void LeaveGame();  UFUNCTION(BlueprintCallable, Category="CL9|Session") bool JoinGame(const FString& Address);  UFUNCTION(BlueprintCallable, Category="CL9|Session") void CancelJoin();  UFUNCTION(BlueprintPure, Category="CL9|Session") bool IsJoining() const;  UFUNCTION(BlueprintPure, Category="CL9|Session") bool IsJoinAvailable() const;  UPROPERTY(Config) FSoftObjectPath StartLevel;  // HostGame: registry site -> Level long package name -> UGameplayStatics::OpenLevel(..., true, listen only in !UE_BUILD_SHIPPING), false + log if no Level. LeaveGame: OpenLevel(StartLevel) else GetGameDefaultMap. JoinGame: first local controller ClientTravel(Address, TRAVEL_Absolute) in !UE_BUILD_SHIPPING, false in Shipping. CancelJoin: GetEngine()->CancelPending(*GetWorldContext()). IsJoining: GetWorldContext()->PendingNetGame != nullptr. IsJoinAvailable: !UE_BUILD_SHIPPING`
- **ACL9MenuGameMode** — owner: gameplay (Run B); lead sets it as the Lvl_Start World Settings override — `C:/Work/CL9/Source/CL9/GameMode/CL9MenuGameMode.h/.cpp`
  - `UCLASS() class ACL9MenuGameMode : public AGameModeBase { public: ACL9MenuGameMode(); /* DefaultPawnClass = nullptr; HUDClass = nullptr */  virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override; /* PlayerControllerClass from the global default game mode CDO, then Super */  virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override; /* no pawn */  UPROPERTY(EditDefaultsOnly, Category="UI") FName TitleUIRow = FName("Title"); };`
- **ACL9PlayerController menu hooks** — owner: gameplay only (Run B) — `C:/Work/CL9/Source/CL9/GameMode/CL9PlayerController.h/.cpp`
  - `UFUNCTION(BlueprintPure, Category="CL9|Session") bool IsInMenuWorld() const;  // GetWorld()->GetAuthGameMode<ACL9MenuGameMode>() != nullptr.  BeginPlay (local, after InitForPlayer): if menu -> UIManager->OpenRootScreen(MenuGM->TitleUIRow).  DebugPanelInput: return if IsInMenuWorld().  PauseInput unchanged`
- **ECL9DepartResult / FCL9RunOver** — owner: gameplay (Run B) — `C:/Work/CL9/Source/CL9/Data/CL9Types.h`
  - `UENUM(BlueprintType) enum class ECL9DepartResult : uint8 { Ok, NotHost, NotLobby, OrdersPending, AlreadyDeparting };  USTRUCT(BlueprintType) struct FCL9RunOver { GENERATED_BODY() UPROPERTY(BlueprintReadOnly) bool bActive = false; UPROPERTY(BlueprintReadOnly) int32 Day = 0; UPROPERTY(BlueprintReadOnly) int32 BestDay = 0; };`
- **ACL9GameState run-over state** — owner: gameplay provides (Run B); ui HUD and GameOver widget consume — `C:/Work/CL9/Source/CL9/GameMode/CL9GameState.h/.cpp`
  - `void SetRunOver(int32 Day, int32 BestDay);  /* authority */  UFUNCTION(BlueprintPure, Category="Run") bool IsRunOver() const;  UFUNCTION(BlueprintPure, Category="Run") const FCL9RunOver& GetRunOver() const;  protected: UPROPERTY(Replicated, BlueprintReadOnly, Category="Run") FCL9RunOver RunOver;  // DOREPLIFETIME(ACL9GameState, RunOver); no delegate - the HUD polls`
- **ACL9GameMode flow additions** — owner: gameplay (Run B) — `C:/Work/CL9/Source/CL9/GameMode/CL9GameMode.h/.cpp`
  - `ECL9DepartResult RequestDepart(ACL9PlayerController* Requester);  protected: void FinishRunOver();  void SpawnDepartLever();  int32 RecordBestDay(int32 Day); /* now returns the record */  FTimerHandle RunOverHandle;  // StartPlay calls SpawnDepartLever after the existing body; HandlePlayerDeath wipe branch and EndNight changed as in decision L7`
- **ACL9DepartLever** — owner: gameplay (Run B) — `C:/Work/CL9/Source/CL9/Items/CL9DepartLever.h/.cpp`
  - `UCLASS() class ACL9DepartLever : public ACL9Fixture, public ICL9Interactable { public: ACL9DepartLever();  virtual void BeginPlay() override; /* load Rules->DepartLeverMesh locally into MeshComponent */  virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;  virtual FText GetPrompt(const ACL9Character* Who) const override;  virtual bool CanInteract(const ACL9Character* Who) const override;  virtual void Interact(ACL9Character* Who) override;  UFUNCTION(BlueprintPure, Category="Depart") bool IsArmed() const; /* GameState->GetServerWorldTimeSeconds() < ArmedUntilServerTime */  protected: UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> FocusBox;  UPROPERTY(Replicated) float ArmedUntilServerTime = 0.f; };`
- **UCL9RulesDef Run B columns** — owner: gameplay (Run B) — `C:/Work/CL9/Source/CL9/Data/CL9RulesDef.h/.cpp (documented in docs/plan/data/rules.md)`
  - `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Run") bool bWipeEndsRun = false;  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depart") float DepartArmTime = 5.f;  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depart") TSoftObjectPtr<UStaticMesh> DepartLeverMesh;  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depart") FTransform DepartLeverOffset = FTransform(FRotator(0.f, 90.f, 0.f), FVector(40.f, 150.f, 130.f));  // DepartLeverMesh added to the GameInstance Warm list`
- **Dev console commands (non-Shipping)** — owner: gameplay (Run B) — `C:/Work/CL9/Source/CL9/GameMode/CL9Cheats.cpp and CL9CheatManager.cpp`
  - `cl9.host <siteAlias> (local: GameInstance->HostGame);  cl9.leave (local: GameInstance->LeaveGame);  cl9.join <ip[:port]> (local: GameInstance->JoinGame);  cl9.depart (host: RequestDepart for the host controller, logs the ECL9DepartResult);  cl9.lever (host: calls the level's ACL9DepartLever::Interact once with the host character, logs armed/departed/opened-orders)`
- **UCL9UIManager travel reset and root screens** — owner: ui provides (Run B); gameplay's PlayerController calls OpenRootScreen — `C:/Work/CL9/Source/CL9/UI/CL9UIManager.h/.cpp`
  - `InitForPlayer stale rule: const bool bStale = !IsValid(MainWidget) || OwningPlayer.Get() != PlayerController || !MainWidget->IsInViewport();  UCL9Widget* OpenRootScreen(FName UIName);  /* OpenWidget(UIName, true) and RootScreens.Add(UIName) */  bool IsRootScreen(FName UIName) const;  // CloseWidget(UIName) returns without closing when IsRootScreen(UIName); RootScreens cleared on re-bootstrap; one log line 'UIManager: bootstrapped for <PC> in <World>' per bootstrap`
- **UI names and table rows** — owner: ui (Run B); lead writes the asset — `C:/Work/CL9/Source/CL9/UI/CL9UITable.h; /Game/UI/UITable (lead)`
  - `namespace CL9UINames: inline const FName GameOver(TEXT("GameOver"));  UITable Rows (exact keys, original case): "Title" -> { WBP_Title_C or UCL9TitleWidget, Screen }, "GameOver" -> { WBP_GameOver_C or UCL9GameOverWidget, Screen }`
- **UCL9TitleWidget / UCL9GameOverWidget** — owner: ui (Run B) — `C:/Work/CL9/Source/CL9/UI/CL9TitleWidget.h/.cpp, CL9GameOverWidget.h/.cpp`
  - `class UCL9TitleWidget : public UCL9Widget  // self-building; buttons call UCL9GameInstance::HostGame(alias), UCL9UIManager::OpenWidget(Settings), UKismetSystemLibrary::QuitGame; join section (S6) calls JoinGame / CancelJoin and polls IsJoining.  class UCL9GameOverWidget : public UCL9Widget  // reads ACL9GameState::GetRunOver(); one button -> UCL9GameInstance::LeaveGame(); host/client label by GetWorld()->GetNetMode() != NM_Client`
- **UCL9HUDWidget additions** — owner: ui (Run B) — `C:/Work/CL9/Source/CL9/UI/CL9HUDWidget.h/.cpp`
  - `void DeriveRunOverScreen();  /* called in NativeTick before DeriveOrderScreen: IsRunOver && open screen not in { GameOver, Settings, DebugPanel, Pause } -> UIManager->OpenRootScreen(CL9UINames::GameOver); DeriveOrderScreen returns early while IsRunOver */  ESlateVisibility WiredVisibility;  /* NativeConstruct: WiredVisibility = GetVisibility(); SetVisibility(Collapsed); TryWire success: SetVisibility(WiredVisibility) */  S5 only: bool bOrderScreenAutoOpened = false;`
- **Prompt string keys shared across roles** — owner: ui adds rows; gameplay uses the keys (Run B) — `C:/Work/CL9/Content/Strings/CL9_Strings.csv`
  - `LOCTABLE("CL9", "Prompt.Depart"), "Prompt.DepartConfirm", "Prompt.DepartOrders", "Prompt.DepartHostOnly", "Prompt.DepartSoon" - used as literals by gameplay's lever; rows added by ui`

- **UCL9GameInstance::GetHostableSites** (added by the lead 2026-10-05, must-land) — owner: gameplay provides (Run B); ui Title consumes — `C:/Work/CL9/Source/CL9/GameMode/CL9GameInstance.h/.cpp`
  - `UFUNCTION(BlueprintPure, Category="CL9|Session") TArray<FName> GetHostableSites() const;  // aliases of registry sites whose Level is set, in table order; display name = the site row's DisplayName`

## Acceptance Criteria
- [ ] B0: zero Blueprint compile errors in Saved/Logs/CL9.log after each editor start, checked before any StartPIE
- [ ] B1 regression with bWipeEndsRun false, PIE from Lvl_Flat: boots into Lobby with HUD visible and no Title; OrderSelect opens; cl9.night still starts a night; dying solo during Night behaves exactly as today (EndNight -> next-day lobby); the 8 pre-existing UITable rows are unchanged
- [ ] B1 lever: exactly one ACL9DepartLever exists within 250 cm of the truck pad's +X face in Lvl_Flat and henhouse and does not block the wake spot (player can walk 300 cm forward from spawn); with an offer pending cl9.lever opens OrderSelect and does not arm, cl9.depart logs OrdersPending; after orders are accepted the first cl9.lever arms (IsArmed true), a second within 5 s starts the night, a press after 5 s only re-arms
- [ ] B2 menu (editor level Lvl_Start, PIE): auth game mode is ACL9MenuGameMode; zero ACL9Character and zero ACL9SiteMap actors; HUD collapsed; open screen == Title with the cursor visible; M and Tab leave Title open; Settings opened from Title lists the gameplay key rows and closing it returns to Title
- [ ] B2 host: clicking a stage (or cl9.host henhouse) -> world is Lvl_Henhouse; exactly one 'UIManager: bootstrapped' log line for this travel; main widget in the viewport and owned by the current local controller; OpenWidget on a valid row returns non-null; HUD visible; OrderSelect opens; the log shows the net driver listening; CurrentDay 1, Phase Lobby
- [ ] B2 run over (bWipeEndsRun true): accept orders, cl9.lever twice -> Phase Night; kill the host with cl9.damage -> within SpectateDelay + 0.5 s RunOver.bActive is true, Phase == Dawn, CurrentDay unchanged, no monsters or night-spawned items remain, BestDay >= Day, GameOver is the open screen; cl9.night and the shop are refused; M does not close GameOver; 30 s idle with zero LogCL9 Error lines and no ensure
- [ ] B2 leave and re-host: the GameOver button (or cl9.leave) -> world is Lvl_Start with Title open and one new bootstrap log line; cl9.host henhouse again succeeds (listening a second time, day 1 Lobby, lever present)
- [ ] B2 join API (API only): cl9.join 127.0.0.1 from Title with no host running sets IsJoining true and CancelJoin returns it to false without leaving Title; if S6 landed the same through the Title join section

### Shared gates (both runs)
- [ ] G-FINAL: sources build; editor DLL newer than the newest source; editor launches; PIE on Lvl_Flat starts with no modal; HUD visible; OrderSelect opens; cl9.night starts a night; log error count not above G0; EditorStartupMap and PIE play settings unchanged; locks released; guard never switched off; nothing committed; GameDefaultMap is Lvl_Start only if B2 passed; every binary asset touched has a .before copy listed in morning.md
- [ ] TIME: the hard stops (T+1:35, T+2:10, T+3:45, T+4:40) are acceptance - an overrun item is reverted or left flagged off, not continued
- [ ] REPORT: morning.md exists and its first 20 lines carry the two verdicts with the measured table, the one-minute try-it steps, the decisions taken on the owner's behalf, the unverified list and the off-switches
- [ ] DEFERRED (needs a joining client; reported unverified, never claimed): client exhaustion at 6.0 +-0.05 s on the same move as the host at t.MaxFPS 144; speed non-decreasing after sprint press on a 120 fps client; '*** Client: Error' count with p.NetShowCorrections 1 over a scripted course at 60 and 144 fps not above baseline; coyote/jump-buffer on a client; the other machine hears none of the new sounds and the host's copy of a client pawn keeps camera relative Z == EyeZ; client GameOver/leave, lever prompts on clients, join by address, host loss landing on Title

## Related Files / Modules
| File | Role |
|---|---|
| `Source/CL9/GameMode/CL9GameInstance.{h,cpp}` | Session API (`HostGame`, `LeaveGame`, `JoinGame`, ...), config `StartLevel` |
| `Source/CL9/GameMode/CL9MenuGameMode.{h,cpp}` | New native menu game mode for `Lvl_Start` |
| `Source/CL9/GameMode/CL9GameMode.{h,cpp}` | `RequestDepart`, lever spawn, run-over timer / `FinishRunOver`, `EndNight` guard |
| `Source/CL9/GameMode/CL9GameState.{h,cpp}` | Replicated `FCL9RunOver` |
| `Source/CL9/GameMode/CL9PlayerController.{h,cpp}` | `IsInMenuWorld`, Title opening, debug-panel guard (single writer: gameplay) |
| `Source/CL9/Items/CL9DepartLever.{h,cpp}` | New interactable |
| `Source/CL9/Data/CL9RulesDef.{h,cpp}`, `CL9Types.h` | `bWipeEndsRun`, depart columns, `ECL9DepartResult`, `FCL9RunOver` |
| `Source/CL9/GameMode/CL9Cheats.cpp`, `CL9CheatManager.*` | `cl9.host`, `cl9.leave`, `cl9.join`, `cl9.depart`, `cl9.lever` |
| `Source/CL9/UI/CL9UIManager.{h,cpp}`, `CL9UITable.h` | Stale-widget re-bootstrap, root screens, `CL9UINames::GameOver` |
| `Source/CL9/UI/CL9TitleWidget.{h,cpp}`, `CL9GameOverWidget.{h,cpp}` | New self-building screens |
| `Source/CL9/UI/CL9HUDWidget.{h,cpp}` | `DeriveRunOverScreen`, self-hide until wired |
| `Content/Strings/CL9_Strings.csv` | Rows from No 208 |
| Lead-only assets | `Content/Maps/Lvl_Start.umap`, `Content/UI/WBP_Title`, `WBP_GameOver`, `Content/UI/UITable` (+2 rows), `Content/Data/CL9Rules` (lever mesh, `bWipeEndsRun`), `Config/DefaultGame.ini` (`StartLevel`), `Config/DefaultEngine.ini` (`GameDefaultMap`, last and only if Gate B2 passed) |

## Must-Preserve
- `StartNight` / `EndNight` / `EnterLobby`, `ECL9NightPhase`, `OnNightPhaseChanged` (steam-release.md Must-Preserve); no new phase value.
- Opening a stage level directly (PIE on `Lvl_Flat`, or `-game <stage map>`) behaves exactly as today while `bWipeEndsRun` is false: Lobby, HUD, order screen, `cl9.night`, wipe -> next day.
- The 8 existing UITable rows; `WBP_Main`, `WBP_HUD`, `WBP_Pause`, input assets and `BP_CL9PlayerController` are not edited; the four stage `.umap` files are not edited.
- `EditorStartupMap` stays `Lvl_Flat`; PIE play settings unchanged.
- Settlement failure keeps its current in-place restart; the autosave keeps its current behaviour (not deleted on run over).
- `session.md:200` "no direct-IP join in the product": listening and `JoinGame` are compiled only in non-Shipping builds — this is the development transport until the Steam layer (steam-release W1/W2) replaces the inside of the same three GameInstance functions.

## Fairness Constraints
- Departure, run-over and order confirmation are host rulings (GameMode); clients only see replicated state.
- `bWipeEndsRun` reverses the 2026-09 decision "wipe -> immediate EndNight" on the owner's explicit instruction; it stays a data switch so the old rule is one cell away.

## Existing Integration Contract
- The HUD derives screens from state every tick (`DeriveOrderScreen` precedent); GameOver follows the same pattern and yields to Settings / DebugPanel / Pause.
- Screens are opened only through `UCL9UIManager` + UITable rows; root screens are a manager concept, `PauseInput` is not edited.
- Interactables go through `ICL9Interactable` (`CanInteract` gates the prompt; `Interact` runs on authority).

## Execution Notes
- Recommended model: the strongest current Claude model (Claude Fable 5.1) for the lead (asset work, gates, the travel/UI verification) and for review; Claude Opus for both role agents (gameplay: flow + replication; ui: manager re-bootstrap and self-building widgets are new patterns here).
- This document cannot enforce the model — the executing session's `/model` setting decides. If the session model is below the recommendation, surface it to the user and confirm before proceeding.
- Order, compile gate, safe-failure rules and time boxes: see "Execution plan" in [control-feel.md](control-feel.md).

## Results (2026-10-05, run start-lobby)
- Builds: #3 green on the first try (98 s), #4 after the review fix batch (57 s). Nothing committed.
- Landed: every must-land item plus stretch S5 (order screen auto-opens once per offer) and S6 (Title join section). Zero new Blueprints with graphs: `Lvl_Start` (empty level, native `ACL9MenuGameMode` override), two empty-shell widget blueprints (`WBP_Title`, `WBP_GameOver`), UITable +2 rows (8 old rows unchanged), `CL9Rules.DepartLeverMesh`, `Config/DefaultGame.ini` `StartLevel`, 23 string rows (No 208-230).
- Verified in PIE as a solo host, on the final build:
  - B1 (wipe switch off, PIE on `Lvl_Flat`): Lobby + HUD + order screen as before; one lever 240 cm from the truck pad, not blocking the spawn (walked 598 cm forward); with an offer pending the lever re-opens the order screen and `cl9.depart` reports OrdersPending; after `cl9.pick` the first press arms, the second starts the night; a solo death runs the old path (EndNight -> next day -> existing settlement).
  - B2 (wipe switch on, PIE from `Lvl_Start`): menu game mode, no character, HUD collapsed, Title open with the cursor; M / Tab leave it open; Settings over Title and back; hosting henhouse travels, re-bootstraps the UI once and listens on 7777; orders -> lever twice -> Night -> death -> "game over in 3.0s" -> night population cleared -> Phase Dawn held, day unchanged, `RunOver{active, day 1, best 2}`, GameOver screen; `cl9.night` and the lever refuse; 30 s idle with zero errors; the GameOver button returns to `Lvl_Start` / Title; hosting again gives a clean day-1 lobby; a pending join is cancelled by hosting and nothing pulls the player away 65 s later.
  - Join API: `cl9.join 127.0.0.1` -> joining yes -> cancel -> joining no, still on Title.
  - Standalone (`UnrealEditor.exe CL9.uproject -game`): boots into `/Game/Maps/Lvl_Start` with `CL9MenuGameMode` and the Title.
  - G-FINAL: PIE on `Lvl_Flat` starts with no modal, HUD visible, order screen opens, `cl9.night` starts a night, LogCL9 errors 0, `EditorStartupMap` unchanged.
- Switch states left on: `CL9Rules.bWipeEndsRun` cell = **true** (header default still false), `GameDefaultMap` = `/Game/Maps/Lvl_Start`, `StartLevel` = `/Game/Maps/Lvl_Start`.
- Review (2 lenses + refutation pass): no blocker, no major. Fixed in build #4: pending join cancelled before hosting/leaving, swallowed night expiry re-issued if a run-over is cancelled, lever display name (`Facility.DepartLever`), order-screen re-arm only when the offer itself changes, GameOver open-retry latch.
- **Unverified - needs a second player** (never claimed as working): a real join by address, clients' view of the lever prompts / GameOver / leave, what clients see when the host leaves, mid-night join behaviour. Not built tonight (see Out-of-scope): join gate and 4-player cap, roster, failure notices, pause-menu leave button.
- Not run: `CL9.Replication.Registration` (it is known to crash the editor when run in-editor; the review confirmed by reading that the two new replicated fields are registered).
