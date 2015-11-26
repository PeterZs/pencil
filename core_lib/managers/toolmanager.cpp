
#include "pentool.h"
#include "penciltool.h"
#include "brushtool.h"
#include "buckettool.h"
#include "erasertool.h"
#include "eyedroppertool.h"
#include "handtool.h"
#include "movetool.h"
#include "polylinetool.h"
#include "selecttool.h"
#include "smudgetool.h"
#include "toolmanager.h"
#include "editor.h"
#include "pencilsettings.h"


ToolManager::ToolManager(QObject* parent ) 
    : BaseManager( parent )
    , m_pCurrentTool( nullptr )
    , m_eTabletBackupTool( INVALID_TOOL )
{
}

bool ToolManager::init()
{
    isSwitchedToEraser = false;

    m_toolSetHash.insert( PEN, new PenTool );
    m_toolSetHash.insert( PENCIL, new PencilTool );
    m_toolSetHash.insert( BRUSH, new BrushTool );
    m_toolSetHash.insert( ERASER, new EraserTool );
    m_toolSetHash.insert( BUCKET, new BucketTool );
    m_toolSetHash.insert( EYEDROPPER, new EyedropperTool );
    m_toolSetHash.insert( HAND, new HandTool );
    m_toolSetHash.insert( MOVE, new MoveTool );
    m_toolSetHash.insert( POLYLINE, new PolylineTool );
    m_toolSetHash.insert( SELECT, new SelectTool );
    m_toolSetHash.insert( SMUDGE, new SmudgeTool );

    foreach( BaseTool* pTool, m_toolSetHash.values() )
    {
        pTool->initialize( editor() );
    }

    setDefaultTool();

    return true;
}

Status ToolManager::onObjectLoaded( Object* )
{
    return Status::OK;
}

BaseTool* ToolManager::getTool(ToolType eToolType)
{
    return m_toolSetHash[ eToolType ];
}

void ToolManager::setDefaultTool()
{
    // Set default tool
    // (called by the main window init)
    ToolType defaultToolType = PENCIL;

    setCurrentTool(defaultToolType);
    m_eTabletBackupTool = defaultToolType;
}

void ToolManager::setCurrentTool( ToolType eToolType )
{
    m_pCurrentTool = getTool( eToolType );
    Q_EMIT toolChanged( eToolType );
}

void ToolManager::cleanupAllToolsData()
{
    foreach ( BaseTool* pTool, m_toolSetHash.values() )
    {
        pTool->clear();
    }
}

void ToolManager::resetAllTools()
{
    // Reset can be useful to solve some pencil settings problems.
    // Betatesters should be recommended to reset before sending tool related issues.
    // This can prevent from users to stop working on their project.
    getTool( PEN )->properties.width = 1.5; // not supposed to use feather
    getTool( POLYLINE )->properties.width = 1.5; // PEN dependent
    getTool( PENCIL )->properties.width = 1.0;
    getTool( PENCIL )->properties.feather = -1.0; // locks feather usage (can be changed)
    getTool( ERASER )->properties.width = 25.0;
    getTool( ERASER )->properties.feather = 50.0;
    getTool( BRUSH )->properties.width = 15.0;
    getTool( BRUSH )->properties.feather = 200.0;
    getTool( SMUDGE )->properties.width = 25.0;
    getTool( SMUDGE )->properties.feather = 200.0;

    pencilSettings()->setValue( SETTING_TOOL_CURSOR, true );
    // todo: add all the default settings

    qDebug( "tools restored to default settings" );
}

void ToolManager::setWidth( float newWidth )
{
    currentTool()->setWidth(newWidth);
    Q_EMIT penWidthValueChanged( newWidth );
    Q_EMIT toolPropertyChanged( currentTool()->type(), WIDTH );
}

void ToolManager::setFeather( float newFeather )
{
    currentTool()->setFeather(newFeather);
    Q_EMIT penFeatherValueChanged( newFeather );
    Q_EMIT toolPropertyChanged( currentTool()->type(), FEATHER );
}

void ToolManager::setInvisibility( bool isInvisible )
{
    currentTool()->setInvisibility(isInvisible);
    Q_EMIT toolPropertyChanged( currentTool()->type(), INVISIBILITY );
}

void ToolManager::setPreserveAlpha( bool isPreserveAlpha )
{
    currentTool()->setPreserveAlpha(isPreserveAlpha);
    Q_EMIT toolPropertyChanged( currentTool()->type(), PRESERVEALPHA );
}

void ToolManager::setPressure( bool isPressureOn )
{
    currentTool()->setPressure( isPressureOn );
    Q_EMIT toolPropertyChanged( currentTool()->type(), PRESSURE );
}

void ToolManager::tabletSwitchToEraser()
{
    if (!isSwitchedToEraser)
    {
        isSwitchedToEraser = true;

        m_eTabletBackupTool = m_pCurrentTool->type();
        setCurrentTool( ERASER );
    }
}

void ToolManager::tabletRestorePrevTool()
{
    if ( isSwitchedToEraser )
    {
        isSwitchedToEraser = false;
        if ( m_eTabletBackupTool == INVALID_TOOL )
        {
            m_eTabletBackupTool = PENCIL;
        }
        setCurrentTool( m_eTabletBackupTool );
    }
}
