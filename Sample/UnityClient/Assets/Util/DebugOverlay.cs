using UnityEngine;
using System.Collections;
using System.Collections.Generic;


/**
 * A singleton which facilitates writing debug annotations to the screen.
 * 
 * Features:
 * - write text in the viewport for posting messages/errors to the user
 * - write text in world space
 * 
 * TODO:
 * - lines (Persistent ones which is different than Debug.DrawLine())
 * - rays (Persistent ones which is different than Debug.DrawRay())
 * - spheres
 * - boxes
 */
public class DebugOverlay : MonoBehaviourSingleton<DebugOverlay>
{
    /**
     * Base class for any element.
     */
    protected class ElementBase
    {
        public float time = 0;                                  //!< The amount of time to display the item.  If -1 then it is persistent and must be cleared explicitly.
        public Color color = new Color(1, 1, 1, 1);             //!< The color in which to render the item.
        public Color shadowColor = new Color(0, 0, 0, 0.5f);    //!< The color in which to render the item.
    }

    /**
     * Element which represents a piece of viewport text.
     */
    protected class ViewportTextElement : ElementBase
    {
        public string[] text = new string[0];
		public int fontSize = 0;
    }

    /**
     * Element which represents a piece of text in world space.
     */
    protected class WorldSpaceTextElement : ElementBase
    {
        public string[] text = new string[0];
        public Vector3 pos = Vector3.zero;
        public int fontSize = 12;
    }

    protected List<ViewportTextElement> m_viewportTextElements = new List<ViewportTextElement>();   //!< The list of text elements to render in viewport space.
    protected List<WorldSpaceTextElement> m_worldTextElements = new List<WorldSpaceTextElement>();  //!< The list of text elements to render in world space.
    //protected int m_viewportFontSize = 26; 														    //!< The default font size for rendering in viewport space.
    protected int m_viewportFontSize = 12; 														    //!< The default font size for rendering in viewport space.
	protected int m_viewportTextSpacing = 2; 											            //!< The line spacing between viewport text elements.
    protected float m_errorTime = 0;                                                                //!< The number of seconds to display the error flash.

    [SerializeField]
    protected bool m_renderErrors = true;                                                           //!< Adds a flash to the viewport when an error occurs.

    /**
     * Flash that there's an error in the UI.
     */
    public void NotifyError()
    {
        if (!m_renderErrors)
        {
            return;
        }

        m_errorTime = 3;
    }

    /**
     * Update the state of elements.
     */
    public void Update()
    {
        UpdateElementList(m_viewportTextElements);
        UpdateElementList(m_worldTextElements);
		
		if (m_errorTime > 0)
		{
			m_errorTime = Mathf.Max(0, m_errorTime - Time.deltaTime);
		}
    }

    protected void UpdateElementList<T>(List<T> list)
    {
        // update viewport text elements
        for (int i = 0; i < list.Count; )
        {
            ElementBase element = list[i] as ElementBase;

            // only check elements which are not persistent
            if (element.time >= 0)
            {
                element.time -= Time.deltaTime;

                if (element.time <= 0)
                {
                    list.RemoveAt(i);
                    continue;
                }
            }

            ++i;
        }
    }

    #region Element Creation

    #region Viewport Text

    /**
     * Adds text lines to the viewport.
     * 
     * @param text The lines of text to render.
     * @param time The time before fading.
     * @param color The color to use.
     */
    public void AddViewportText(string[] text, float time, Color color)
    {
        ViewportTextElement element = new ViewportTextElement();
        element.color = color;
        element.text = text;
        element.time = time;

        m_viewportTextElements.Add(element);
    }

    /**
     * Adds text to the viewport.
     * 
     * @param text The text to add.
     * @param time The number of seconds to keep it in the viewport.  If negative then it will remain until cleared.
     * @param color The color of the text.
     */
    public void AddViewportText(string text, float time, Color color)
    {
        AddViewportText(new string[] { text }, time, color);
    }

    /**
     * Adds text to the viewport.
     * 
     * @param text The text to add.
     * @param time The number of seconds to keep it in the viewport.  If negative then it will remain until cleared.
     */
    public void AddViewportText(string text, float time)
    {
        AddViewportText(text, time, Color.green);
    }

    /**
     * Adds text to the viewport which fades after a few seconds.
     * 
     * @param text The text to add.
     */
    public void AddViewportText(string text)
    {
        AddViewportText(text, 10.0f);
    }

    /**
     * Clears all text from the viewport.
     */
    public void ClearViewportText()
    {
        m_viewportTextElements.Clear();
    }

    #endregion

    #region World Space Text

    /**
     * Adds text lines to the world.
     * 
     * @param text The lines of text to add.
     * @param time The number of seconds to keep it in the world.  If negative then it will remain until cleared.
     * @param color The color of the text.
     * @param fontSize The size of the font.
     */
    public void AddWorldText(string[] text, Vector3 pos, float time, Color color, int fontSize)
    {
        WorldSpaceTextElement element = new WorldSpaceTextElement();
        element.color = color;
        element.text = text;
        element.time = time;
        element.pos = pos;
        element.fontSize = fontSize;

        m_worldTextElements.Add(element);
    }

    /**
     * Adds text to the world.
     * 
     * @param text The text to add.
     * @param time The number of seconds to keep it in the world.  If negative then it will remain until cleared.
     * @param color The color of the text.
     */
    public void AddWorldText(string text, Vector3 pos, float time, Color color, int fontSize)
    {
        AddWorldText(new string[] { text }, pos, time, color, fontSize);
    }

    /**
     * Adds text to the world.
     * 
     * @param text The text to add.
     * @param time The number of seconds to keep it in the viewport.  If negative then it will remain until cleared.
     */
    public void AddWorldText(string text, Vector3 pos, float time)
    {
        AddWorldText(text, pos, time, Color.green, 12);
    }

    /**
     * Adds text to the world which fades after a few seconds.
     * 
     * @param text The text to add.
     */
    public void AddWorldText(string text, Vector3 pos)
    {
        AddWorldText(text, pos, 3.0f);
    }

    /**
     * Clears all text from the world.
     */
    public void ClearWorldText()
    {
        m_worldTextElements.Clear();
    }

    #endregion

    #endregion

    #region Rendering

    /**
     * Render all the elements.
     */
    public virtual void OnGUI()
    {
        // add the temporary error flash
        if (m_errorTime > 0)
        {
            AddViewportText("Error!", 1, m_errorTime > 1.5 ? Color.white : Color.red);
			m_viewportTextElements[m_viewportTextElements.Count - 1].fontSize = 36;
        }

        RenderViewportText();

        // remove the temporary error flash
        if (m_errorTime > 0)
        {
            m_viewportTextElements.RemoveAt(m_viewportTextElements.Count - 1);
        }

        RenderWorldText();
    }

    /**
     * Renders the all the viewport text elements.
     */
    protected void RenderViewportText()
    {
        if (m_viewportTextElements.Count == 0)
        {
            return;
        }

        Rect bounds = new Rect(0, 0, Screen.width, Screen.height);

        GUIStyle style = new GUIStyle();
        style.fontStyle = FontStyle.Bold;
        style.alignment = TextAnchor.UpperLeft;
        style.fontSize = m_viewportFontSize;

        // render each element
        for (int i = 0; i < m_viewportTextElements.Count; ++i)
        {
            ViewportTextElement element = m_viewportTextElements[i];
			
			if (element.fontSize <= 0)
			{
				style.fontSize = m_viewportFontSize;
			}
			else
			{
				style.fontSize = element.fontSize;
			}
			
            // render each line
            for (int j = 0; j < element.text.Length; ++j)
            {
                RenderText(element.text[j], element, bounds, style);

                // move the line cursor down
                bounds.yMin += style.fontSize + m_viewportTextSpacing;
            }
        }
    }

    /**
     * Renders the all the world text elements.
     */
    protected void RenderWorldText()
    {
        if (m_worldTextElements.Count == 0)
        {
            return;
        }

        GUIStyle style = new GUIStyle();
        style.fontStyle = FontStyle.Bold;
        style.alignment = TextAnchor.MiddleCenter;

        // render each element
        for (int i = 0; i < m_worldTextElements.Count; ++i)
        {
            WorldSpaceTextElement element = m_worldTextElements[i];
            style.fontSize = element.fontSize;

            RenderWorldTextElement(element, style);
        }
    }

    /**
     * Render the given text element in world space.
     * 
     * @param element The text element.
     * @param style The style to use when rendering.
     */
    protected void RenderWorldTextElement(WorldSpaceTextElement element, GUIStyle style)
    {
        // project the world point onto the screen
        Vector2 ptScreen = Camera.main.WorldToScreenPoint(element.pos);

        // off the screen
        if (ptScreen.x < 0 || ptScreen.y < 0 || ptScreen.x >= Screen.width || ptScreen.y >= Screen.height)
        {
            return;
        }

        // render a single line
        if (element.text.Length == 1)
        {
            Rect bounds = new Rect(ptScreen.x - 1000, Screen.height - ptScreen.y - 1000, 2000, 2000);
            RenderText(element.text[0], element, bounds, style);
        }
        // render each line
        else
        {
            for (int i = 0; i < element.text.Length; ++i)
            {
                Rect bounds = new Rect(ptScreen.x - 1000, Screen.height - ptScreen.y - 1000 + style.fontSize*i, 2000, 2000);
                RenderText(element.text[i], element, bounds, style);
            }
        }
    }

    /**
     * Low level text rendering which handles shadows.
     * 
     * @param text The text to render.
     * @param element The element to render.
     * @param bounds The location of the text on screen.
     * @param style The style to use when rendering.
     */
    protected void RenderText(string text, ElementBase element, Rect bounds, GUIStyle style)
    {
        float alpha = GetAlpha(element.time);

        // shadow
        bounds.xMin += 2;
        bounds.yMin += 2;
        bounds.xMax += 2;
        bounds.yMax += 2;
        style.normal.textColor = new Color(element.shadowColor.r, element.shadowColor.g, element.shadowColor.b, alpha * 0.5f);
        GUI.Label(bounds, text, style);
        bounds.xMin -= 2;
        bounds.yMin -= 2;
        bounds.xMax -= 2;
        bounds.yMax -= 2;

        // element
        style.normal.textColor = new Color(element.color.r, element.color.g, element.color.b, alpha);
        GUI.Label(bounds, text, style);
    }

    /**
     * Retrieves the alpha value for an element based on the amount of time remaining.
     */
	protected float GetAlpha(float time)
	{
        // anything with a time of 0 is to be rendered only once in the current frame
		if (time >= 1 || time <= 0)
		{
			return 1;
		}
		
		return time;
	}

    #endregion
}
