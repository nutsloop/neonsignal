# Part 3: Features & Implementation

This section covers the implementation details of Neonsignal's key features, including its API design, real-time event streaming with SSE, performance optimizations, and HTTP/2 protocol correctness measures.

## API Design

The Neonsignal server exposes a set of APIs for core functionalities like authentication and real-time data streaming.

### Authentication API

The authentication system is designed around WebAuthn for passwordless authentication. A key consideration in the API design is how authentication failures are handled.

**Behavior:**

-   **Browser/HTML Paths:** For standard web pages, an authentication failure results in a standard HTTP 302 redirect to the login page (`/`).
-   **API Routes:** For API endpoints (e.g., `/api/auth/login/options`, `/api/events`), a redirect is undesirable. Instead, an authentication failure returns an HTTP 500 error with a JSON body indicating the reason for the failure.

**Example Failure Response for API:**

```json
{
  "error": "auth-required"
}
```

This distinction ensures that front-end applications can programmatically handle authentication errors without being forced to follow redirects, which would break the application flow.
