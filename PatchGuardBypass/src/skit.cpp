
namespace PG
{
};

PG::Disabler::Run();
PG::Verifier::Run();

typedef enum _ALERT_MODE
{
    BEFORE_TEST,
    AFTER_TEST
} ALERT_MODE;

typedef VOID (* CALLBACK_FUNCTION) (ALERT_MODE AlertMode);

VOID
OnTriggerAlert(
        ALERT_MODE AlertMode
)
{
    int a;

    switch(AlertMode)
    {
    case BEFORE_TEST:
        DisableAllPatches();
        break;

    case AFTER_TEST:
        EnableAllPatches();
        break;
    };
}

PG::Evader::Run();
PG::Evader::RegisterCallback(&OnTriggerAlert);

